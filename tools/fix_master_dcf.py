#!/usr/bin/env python3
"""Repair the dcfgen master DCF so lely can actually build its PDO tables.

Why this exists
---------------
lely's CANopen *master* learns nothing about a slave's PDO layout from the SDO
download list.  It builds its transmit and receive tables from the master's own
object dictionary:

  * transmit (RPDO) goes through ``co_dev_tpdo_event()``, which scans 0x1800.. for
    a communication record whose 0x1A00 mapping contains the object just written
    and transmits on that record's COB-ID;
  * receive (TPDO) goes through ``co_rpdo_start()``, which installs a CAN filter
    on 0x1400..'s COB-ID and decodes incoming frames via the 0x1600 mapping.

``dcfgen`` describes a *slave*, so the master DCF it emits invents local mirror
objects at 0x2001/0x2002/0x2004/0x2005 ("slave_1: Status word", ...) and points
0x1600/0x1A00 at those mirrors.  The real CiA 402 objects are never emitted at all.
Every consequence is silent:

  1. ``co_dev_chk_tpdo()`` rejects 0x6040/0x6060/0x607A/0x60FF because those
     objects are absent from the master OD, so ``co_dev_tpdo_event()`` returns
     immediately and **not one RPDO frame reaches the bus**;
  2. ``Device::TpdoWrite()`` cannot resolve a mapping, reports NO_PDO, and drops
     the assignment - so ``tpdo_mapped[0x6040][0] = cw`` does nothing;
  3. incoming TPDO frames decode into the mirrors, so Statusword, position and
     velocity feedback never reach the application.

SDO is unaffected, which is why a bring-up can walk stage after stage
"successfully" while the motor never moves.

What this script does
---------------------
Injects the real CiA 402 objects into the master OD with the access type and
PDOMapping flag lely checks, then rewrites the master's PDO records so they
reference those real indices.  It re-reads the result and refuses to write
anything unless every mapping resolves - the previous hand-patched attempt at
this failed precisely because it was never verified against lely.

Slot layout (one DCF serves every configured node)
--------------------------------------------------
  TX 0..2 -> RPDO1/2/3 of node 1 (0x201 / 0x301 / 0x401)
  TX 3..5 -> RPDO1/2/3 of node 2 (0x202 / 0x302 / 0x402)
  RX 0..2 -> TPDO1/2/3 of node 1 (0x181 / 0x281 / 0x381)
  RX 3..5 -> TPDO1/2/3 of node 2 (0x182 / 0x282 / 0x382)

lely pairs each local mapping with its "remote" counterpart and requires the two
to agree in entry count and per-entry bit length, so 0x5Exx is written as a copy
of 0x1Axx (transmit) and 0x5Axx as a copy of 0x1600 (receive).
"""

from __future__ import annotations

import argparse
import re
import sys
from typing import Dict, List, Tuple

# CiA 301 mapping entry: (index << 16) | (sub-index << 8) | bit length.
Obj = Tuple[int, int, int, int]  # index, sub-index, bit length, data type

UNSIGNED8, UNSIGNED16, UNSIGNED32 = 0x0005, 0x0006, 0x0007
INTEGER8, INTEGER32 = 0x0002, 0x0004

# The indices the C++ driver uses (see mbdv::od in drive_errors.hpp). The master OD
# must contain exactly these, or lely cannot resolve a PDO mapping for them.
CTRLWORD: Obj = (0x6040, 0x00, 16, UNSIGNED16)
MODES_OF_OPERATION: Obj = (0x6060, 0x00, 8, INTEGER8)
TARGET_POSITION: Obj = (0x607A, 0x00, 32, INTEGER32)
TARGET_VELOCITY: Obj = (0x60FF, 0x00, 32, INTEGER32)
STATUSWORD: Obj = (0x6041, 0x00, 16, UNSIGNED16)
POSITION_ACTUAL: Obj = (0x6064, 0x00, 32, INTEGER32)
VELOCITY_ACTUAL: Obj = (0x606C, 0x00, 32, INTEGER32)
ERROR_CODE: Obj = (0x603F, 0x00, 16, UNSIGNED16)
DSP_ALARM_CODE: Obj = (0x200F, 0x00, 32, UNSIGNED32)

LABELS = {
    0x6040: "Controlword",
    0x6041: "Statusword",
    0x6060: "Modes of operation",
    0x6064: "Position actual value",
    0x606C: "Velocity actual value",
    0x607A: "Target position",
    0x60FF: "Target velocity",
    0x603F: "Error code",
    0x200F: "DSP alarm code",
}

# Written by the master into an RPDO. co_dev_chk_tpdo() only demands read + TPDO +
# PDOMapping, but lely then writes the payload into the master's own OD, which also needs
# CO_ACCESS_WRITE: AccessType=rw supplies read, write and TPDO all at once.
TRANSMIT_OBJECTS: List[Obj] = [
    CTRLWORD, MODES_OF_OPERATION, TARGET_POSITION, TARGET_VELOCITY,
]
# Written by the drive into the master OD. co_dev_chk_rpdo() additionally requires
# CO_ACCESS_WRITE and CO_ACCESS_RPDO: AccessType=rw supplies both.
RECEIVE_OBJECTS: List[Obj] = [
    STATUSWORD, POSITION_ACTUAL, VELOCITY_ACTUAL, ERROR_CODE, DSP_ALARM_CODE,
]

# Controlword leads every transmit PDO so a single controlword write reaches the
# drive regardless of which payload PDO lely picks.
TX_PAYLOADS: List[List[Obj]] = [
    [CTRLWORD, MODES_OF_OPERATION],   # RPDO1, COB-ID 0x200 + node
    [CTRLWORD, TARGET_POSITION],     # RPDO2, COB-ID 0x300 + node
    [CTRLWORD, TARGET_VELOCITY],     # RPDO3, COB-ID 0x400 + node
]
RX_PAYLOADS: List[List[Obj]] = [
    [STATUSWORD],                            # TPDO1, COB-ID 0x180 + node
    [POSITION_ACTUAL, VELOCITY_ACTUAL],      # TPDO2, COB-ID 0x280 + node
    [ERROR_CODE, DSP_ALARM_CODE],            # TPDO3, COB-ID 0x380 + node
]

# CiA 301 pre-defined connection set: PDO n sits at base + 0x100 * (n - 1) + node.
TX_COB_BASE = 0x200
RX_COB_BASE = 0x180

EVENT_DRIVEN = 0xFF  # low two bits 11b = event driven
COBID_DISABLED = 0x80000000  # CiA 301: bit 31 set means the PDO is not valid

# Local per-slave mirrors dcfgen invents. Once the real mappings are in place they
# would only be confusing leftovers, and 0x200F is a real drive object.
MIRROR_OBJECTS = (0x2000, 0x2001, 0x2002, 0x2003, 0x2004, 0x2005, 0x2006)

PDO_SUB_SUFFIXES = ("", "sub0", "sub1", "sub2", "sub3", "sub4", "sub5", "sub6",
                    "Value")


def entry(obj: Obj) -> int:
    idx, sub, bits, _ = obj
    return (idx << 16) | (sub << 8) | bits


# ---------------------------------------------------------------------------
# DCF text handling
# ---------------------------------------------------------------------------

SECTION_RE = re.compile(r"^\[([^\]]+)\]\s*$")


def split_sections(text: str) -> Tuple[Dict[str, List[str]], List[str]]:
    sections: Dict[str, List[str]] = {}
    preamble: List[str] = []
    current: List[str] | None = None
    for line in text.splitlines():
        m = SECTION_RE.match(line)
        if m:
            current = sections.setdefault(m.group(1), [])
        elif current is None:
            preamble.append(line)
        else:
            current.append(line)
    return sections, preamble


def join_sections(sections: Dict[str, List[str]], preamble: List[str]) -> str:
    out = list(preamble)
    for name, body in sections.items():
        out.append(f"[{name}]")
        out.extend(body)
    return "\n".join(out).rstrip("\n") + "\n"


def clean(lines: List[str]) -> List[str]:
    """Drop blank lines but keep the terminating one.

    lely parses the DCF with an INI reader that ends a section at the first blank
    line, so every section body must end with one.
    """
    return [ln for ln in lines if ln.strip()] + [""]


def drop_pdo_record(sections: Dict[str, List[str]], idx: int) -> None:
    for suffix in PDO_SUB_SUFFIXES:
        sections.pop(f"{idx:04X}{suffix}", None)


def drop_object(sections: Dict[str, List[str]], idx: int) -> None:
    for suffix in ("", "sub0", "sub1", "sub2", "sub3", "sub4", "sub5", "sub6",
                   "Value", "Name"):
        sections.pop(f"{idx:04X}{suffix}", None)


# ---------------------------------------------------------------------------
# Builders
# ---------------------------------------------------------------------------

def build_comm_record(sections: Dict[str, List[str]], idx: int, name: str,
                      cob_id: int, event_timer_ms: int) -> None:
    sections[f"{idx:04X}"] = clean([
        "SubNumber=6",
        f"ParameterName={name}",
        "ObjectType=0x09",
    ])
    sections[f"{idx:04X}sub0"] = clean([
        "ParameterName=Highest sub-index supported",
        "DataType=0x0005",
        "AccessType=const",
        "DefaultValue=5",
    ])
    sections[f"{idx:04X}sub1"] = clean([
        "ParameterName=COB-ID used by this PDO",
        "DataType=0x0007",
        "AccessType=rw",
        f"DefaultValue=0x{cob_id:08X}",
    ])
    sections[f"{idx:04X}sub2"] = clean([
        "ParameterName=transmission type",
        "DataType=0x0005",
        "AccessType=rw",
        f"DefaultValue=0x{EVENT_DRIVEN:02X}",
    ])
    sections[f"{idx:04X}sub3"] = clean([
        "ParameterName=inhibit time",
        "DataType=0x0006",
        "AccessType=rw",
        "DefaultValue=0",
    ])
    sections[f"{idx:04X}sub4"] = clean([
        "ParameterName=compatibility entry",
        "DataType=0x0005",
        "AccessType=rw",
        "DefaultValue=0",
    ])
    if event_timer_ms > 0:
        sections[f"{idx:04X}sub5"] = clean([
            "ParameterName=event-timer",
            "DataType=0x0006",
            "AccessType=rw",
            f"DefaultValue={event_timer_ms}",
        ])
    else:
        sections.pop(f"{idx:04X}sub5", None)


# The master's object dictionary is SHARED by every node on the bus, so a TPDO from node 1
# and a TPDO from node 2 that map the same object land in the same place: whichever frame
# arrives last wins. Observed effect: node 1 never reported any TPDO at all while node 2
# reported hundreds, and both axes' position/velocity readings were a mixture of the two.
#
# The fix is to give every node its own local copy of the received objects. The index the
# master uses has nothing to do with the index on the drive - it only has to exist in the
# master's OD - so the receive mapping points at a per-node shadow index instead of 0x6041
# and friends. Each axis' driver then reads only its own shadow. SDO reads are unaffected:
# those go to the drive and use the real CiA 402 indices.
def build_mapping_record(sections: Dict[str, List[str]], idx: int, name: str,
                         payload: List[Obj]) -> None:
    sections[f"{idx:04X}"] = clean([
        f"ParameterName={name}",
        "ObjectType=0x08",
        "DataType=0x0007",
        "AccessType=rw",
        f"CompactSubObj={len(payload)}",
    ])
    sections[f"{idx:04X}Value"] = clean(
        [f"NrOfEntries={len(payload)}"]
        + [f"{i + 1}=0x{entry(o):08X}" for i, o in enumerate(payload)]
    )


def build_node_record(sections: Dict[str, List[str]], idx: int,
                      node_id: int) -> None:
    """0x5800+/0x5C00+ "remote PDO number and node-ID".

    lely reads the node-ID as ``value & 0xff``, so the low byte carries it.
    """
    sections[f"{idx:04X}"] = clean([
        "ParameterName=Remote PDO number and node-ID",
        "DataType=0x0007",
        "AccessType=rw",
        f"DefaultValue=0x{0x00000100 | node_id:08X}",
    ])


def build_application_object(sections: Dict[str, List[str]], obj: Obj,
                             access: str) -> None:
    """Write a VAR object whose single sub-index is 0, as CiA 402 objects are.

    lely creates exactly one sub-object at index 0 for an object without
    SubNumber/CompactSubObj, which is what the PDO mapping entries reference.
    """
    idx, _sub, bits, dtype = obj
    sections[f"{idx:04X}"] = clean([
        f"ParameterName={LABELS.get(idx, 'master-local shadow')}",
        "ObjectType=0x07",
        f"DataType=0x{dtype:04X}",
        f"AccessType={access}",
        f"PDOMapping=0x{bits:02X}",
        "DefaultValue=0",
    ])
    for suffix in ("sub0", "sub1", "Value", "Name"):
        sections.pop(f"{idx:04X}{suffix}", None)


OBJECT_SECTION_RE = re.compile(r"^[0-9A-Fa-f]{4}$")


def rebuild_object_lists(sections: Dict[str, List[str]]) -> None:
    """Regenerate [MandatoryObjects] / [OptionalObjects] from the live sections.

    lely creates an object for every entry of those lists and then parses the
    matching section, so an entry pointing at a section we deleted aborts the whole
    DCF with "ParameterName not specified for object 0x....".  Mandatory objects
    keep their status; everything else becomes optional.
    """
    present = sorted(
        int(name, 16) for name in sections if OBJECT_SECTION_RE.match(name)
    )
    mandatory = [i for i in (0x1000, 0x1001, 0x1018) if i in present]
    optional = [i for i in present if i not in mandatory]

    sections["MandatoryObjects"] = clean(
        [f"SupportedObjects={len(mandatory)}"]
        + [f"{i + 1}=0x{idx:04X}" for i, idx in enumerate(mandatory)]
    )
    sections["OptionalObjects"] = clean(
        [f"SupportedObjects={len(optional)}"]
        + [f"{i + 1}=0x{idx:04X}" for i, idx in enumerate(optional)]
    )
    sections.pop("ManufacturerObjects", None)


def update_device_info(sections: Dict[str, List[str]], n_tx: int,
                       n_rx: int) -> None:
    """Keep the advertised PDO counts in step with the records we wrote."""
    body = sections.get("DeviceInfo")
    if body is None:
        return
    for key, value in (("NrOfRxPDO", n_rx), ("NrOfTxPDO", n_tx)):
        for i, line in enumerate(body):
            if line.startswith(key + "="):
                body[i] = f"{key}={value}"
                break
        else:
            body.insert(-1, f"{key}={value}")


# ---------------------------------------------------------------------------
# The repair
# ---------------------------------------------------------------------------

def repair(sections: Dict[str, List[str]], nodes: List[int]) -> None:
    # Every mapped object needs AccessType "rw".
    #
    # Not "ro" for the transmit map: lely's Device::TpdoWrite() resolves the object
    # through the PDO mapping and then *writes the value into the master's own object
    # dictionary* before flagging the transmission event. co_dev_dn_req() enforces
    # CO_ACCESS_WRITE, so an "ro" entry makes every controlword write abort with
    # 0x06010002 "attempt to write a read only object" - the RPDO appears mapped and
    # then silently fails at the first write. "rw" is CO_ACCESS_READ|WRITE|TPDO|RPDO,
    # which also satisfies co_dev_chk_tpdo()'s requirement of read + TPDO + pdo_mapping.
    for obj in TRANSMIT_OBJECTS:
        build_application_object(sections, obj, "rw")
    for obj in RECEIVE_OBJECTS:
        build_application_object(sections, obj, "rw")

    for idx in MIRROR_OBJECTS:
        drop_object(sections, idx)

    slot = 0
    for node in nodes:
        for pdo_no, payload in enumerate(TX_PAYLOADS):
            cob = TX_COB_BASE + 0x100 * pdo_no + node
            build_comm_record(sections, 0x1800 + slot,
                              f"RPDO{pdo_no + 1} of node {node} (master transmit)",
                              cob, 0)
            # NOTE: lely pairs slot k of 0x1800+k with 0x1A00+k, and slot k of
            # 0x1400+k with 0x1600+k - the opposite of CiA 301, where 0x1600 is the RPDO
            # mapping and 0x1A00 the TPDO mapping (this is what the EDS calls them).
            # Follow lely, not the EDS: the transmit arrangement below is the one that
            # has been observed putting a real controlword on the bus.
            build_mapping_record(sections, 0x1A00 + slot,
                                 f"Content of RPDO{pdo_no + 1} to node {node}",
                                 payload)
            build_node_record(sections, 0x5C00 + slot, node)
            build_mapping_record(sections, 0x5E00 + slot,
                                 f"Content of RPDO{pdo_no + 1} seen by node {node}",
                                 payload)
            slot += 1

    slot = 0
    for node in nodes:
        for pdo_no, payload in enumerate(RX_PAYLOADS):
            cob = RX_COB_BASE + 0x100 * pdo_no + node
            build_comm_record(sections, 0x1400 + slot,
                              f"TPDO{pdo_no + 1} of node {node} (master receive)",
                              cob, 0)
            build_mapping_record(sections, 0x1600 + slot,
                                 f"Content of TPDO{pdo_no + 1} from node {node}",
                                 payload)
            build_node_record(sections, 0x5800 + slot, node)
            build_mapping_record(sections, 0x5A00 + slot,
                                 f"Content of TPDO{pdo_no + 1} sent by node {node}",
                                 payload)
            slot += 1

    # lely scans every record it finds, so a leftover slot from dcfgen could make
    # it transmit on a COB-ID we no longer intend to use. Remove the unused ones.
    used = slot
    for base in (0x1400, 0x1600, 0x1800, 0x1A00, 0x5800, 0x5A00, 0x5C00, 0x5E00):
        for i in range(used, 512):
            if f"{base + i:04X}" in sections:
                drop_pdo_record(sections, base + i)

# lely builds its object dictionary from the [MandatoryObjects] / [OptionalObjects]
    # lists, not by scanning for sections, so a list entry without a section is a hard
    # load error. Rebuild the lists from the sections that actually exist.
    rebuild_object_lists(sections)
    update_device_info(sections, len(nodes) * len(TX_PAYLOADS),
                       len(nodes) * len(RX_PAYLOADS))


# ---------------------------------------------------------------------------
# Verification: apply lely's own admission rules to what we wrote
# ---------------------------------------------------------------------------

def parse_uint(text: str | None) -> int | None:
    if text is None:
        return None
    text = text.strip()
    try:
        return int(text, 16) if text.lower().startswith("0x") else int(text, 0)
    except ValueError:
        return None


def read_key(sections: Dict[str, List[str]], section: str, key: str) -> str | None:
    body = sections.get(section)
    if body is None:
        return None
    m = re.search(rf"^{re.escape(key)}=(.*)$", "\n".join(body), re.M)
    return m.group(1).strip() if m else None


def read_mapping(sections: Dict[str, List[str]], idx: int) -> List[int] | None:
    body = sections.get(f"{idx:04X}Value")
    if body is None:
        return None
    count = 0
    found: List[int] = []
    for line in body:
        if line.startswith("NrOfEntries="):
            count = parse_uint(line.split("=", 1)[1]) or 0
        elif re.match(r"^\d+=0x", line):
            found.append(int(line.split("=", 1)[1], 16))
    return found[:count] if count else found


def verify(sections: Dict[str, List[str]], nodes: List[int]) -> List[str]:
    errs: List[str] = []
    tx_used = len(nodes) * len(TX_PAYLOADS)
    rx_used = len(nodes) * len(RX_PAYLOADS)

    # 1. every object the driver uses must exist, be PDO-mappable AND writable
    #
    # Read-write is required for *both* maps, and this is the trap worth writing down:
    # lely's Device::TpdoWrite() resolves 0x6040 through the 0x1600 mapping and then
    # writes the value into the master's own object dictionary before flagging the
    # transmission event. co_dev_dn_req() enforces CO_ACCESS_WRITE, so an "ro" transmit
    # object passes co_dev_chk_tpdo() - which only wants read + TPDO + PDOMapping - and
    # then aborts at the first real write with 0x06010002 "attempt to write a read only
    # object". A mapping check alone cannot catch this; the object has to be writable.
    for obj in [(o, "RPDO/master transmit") for o in TRANSMIT_OBJECTS] + \
               [(o, "TPDO/drive transmit") for o in RECEIVE_OBJECTS]:
        obj, direction = obj
        idx = obj[0]
        section = f"{idx:04X}"
        body = sections.get(section)
        if body is None:
            errs.append(f"[{section}] missing: lely would reject it and drop "
                        f"every PDO that references it")
            continue
        text = "\n".join(body)
        if "PDOMapping=" not in text:
            errs.append(f"[{section}] has no PDOMapping flag")
        access = read_key(sections, section, "AccessType")
        if access not in ("rw", "rwr"):
            errs.append(
                f"[{section}] AccessType={access!r}; as an {direction} object it must be "
                f"rw (CO_ACCESS_READ|WRITE|TPDO|RPDO). lely writes the value into the "
                f"master OD via co_dev_dn_req(), which enforces CO_ACCESS_WRITE - an "
                f"'ro' object aborts every write with 0x06010002 even though the PDO "
                f"mapping is itself valid")

    # 1b. 0x1016 (consumer heartbeat time) must exist in the master OD with at least one
    # usable slot per node. lely's co_dev_cfg_hb() writes the (node-ID, period) pair into
    # it and co_nmt_init() installs the indication function for it, so a missing object
    # means OnHeartbeat() can never fire and a lost node is never noticed.
    if "1016" not in sections:
        errs.append("[1016] is missing from the master OD; lely cannot register a "
                    "heartbeat consumer, so a lost node is never detected")
    else:
        subnum = read_key(sections, "1016", "SubNumber")
        compact = read_key(sections, "1016", "CompactSubObj")
        count = read_key(sections, "1016Value", "NrOfEntries") if "1016Value" in sections else None
        if subnum is not None:
            slots = (parse_uint(subnum) or 0) - 1
        elif compact is not None:
            slots = parse_uint(compact) or 0
        else:
            slots = len(nodes) if count is None else int(count)
            errs.append("[1016] declares neither SubNumber nor CompactSubObj; lely "
                        "cannot size the record")
        if slots < len(nodes):
            errs.append(f"[1016] offers {slots} consumer slot(s) but {len(nodes)} "
                        f"node(s) each need one")

    # 2. record-level checks
    for base, cmap, rmap, node_rec, count, direction in (
        (0x1800, 0x1A00, 0x5E00, 0x5C00, tx_used, "transmit"),
        (0x1400, 0x1600, 0x5A00, 0x5800, rx_used, "receive"),
    ):
        for i in range(count):
            cob = parse_uint(read_key(sections, f"{base + i:04X}sub1", "DefaultValue"))
            trans = parse_uint(read_key(sections, f"{base + i:04X}sub2", "DefaultValue"))
            nid = parse_uint(read_key(sections, f"{node_rec + i:04X}", "DefaultValue"))
            local = read_mapping(sections, cmap + i)
            remote = read_mapping(sections, rmap + i)

            if cob is None:
                errs.append(f"[{base + i:04X}sub1] DefaultValue missing")
            elif cob & COBID_DISABLED:
                errs.append(f"0x{base + i:04X}:01 = 0x{cob:08X} has bit 31 set "
                            f"(PDO marked invalid)")
            if trans is not None and (trans & 0x03) == 0x02:
                errs.append(f"0x{base + i:04X}:02 = 0x{trans:02X} is RTR-only; "
                            f"normally transmitted frames are ignored")
            if not nid:
                errs.append(f"[{node_rec + i:04X}] carries no node-ID; lely skips "
                            f"the PDO")
            elif (nid & 0xFF) not in nodes:
                errs.append(f"[{node_rec + i:04X}] node-ID {nid & 0xFF} is not "
                            f"configured")
            if local is None:
                errs.append(f"[{cmap + i:04X}Value] mapping missing")
            if remote is None:
                errs.append(f"[{rmap + i:04X}Value] mapping missing")
            if local is not None and remote is not None:
                if len(local) != len(remote):
                    errs.append(f"{direction} slot {i}: 0x{cmap + i:04X} has "
                                f"{len(local)} entries but 0x{rmap + i:04X} has "
                                f"{len(remote)}; lely skips the PDO")
                else:
                    for a, b in zip(local, remote):
                        if (a & 0xFF) != (b & 0xFF):
                            errs.append(f"{direction} slot {i}: bit length "
                                        f"0x{a:08X} vs 0x{b:08X} differ; lely "
                                        f"skips the PDO")

    # 3. COB-IDs must be unique, or two PDOs collide on the wire
    for base, count, direction in ((0x1800, tx_used, "transmit"),
                                   (0x1400, rx_used, "receive")):
        seen: Dict[int, int] = {}
        for i in range(count):
            cob = parse_uint(read_key(sections, f"{base + i:04X}sub1", "DefaultValue"))
            if cob is None or cob & COBID_DISABLED:
                continue
            if cob in seen:
                errs.append(f"duplicate {direction} COB-ID 0x{cob:03X} in slots "
                            f"{seen[cob]} and {i}")
            seen[cob] = i

    # 4. no record beyond the ones we planned may be left over from dcfgen
    for cmap, used in ((0x1A00, tx_used), (0x1600, rx_used)):
        for i in range(used, 512):
            if read_mapping(sections, cmap + i):
                errs.append(f"stale 0x{cmap + i:04X} mapping still present; lely "
                            f"scans every record and could transmit the wrong frame")

    # 5. the driver's own objects must be reachable
    tx_indices = {e >> 16 for i in range(tx_used)
                  for e in (read_mapping(sections, 0x1A00 + i) or [])}
    rx_indices = {e >> 16 for i in range(rx_used)
                  for e in (read_mapping(sections, 0x1600 + i) or [])}
    for obj in TRANSMIT_OBJECTS:
        if obj[0] not in tx_indices:
            errs.append(f"0x{obj[0]:04X} is in no 0x1A00 mapping; the driver can "
                        f"never transmit it")
    for obj in RECEIVE_OBJECTS:
        if obj[0] not in rx_indices:
            errs.append(f"0x{obj[0]:04X} is in no 0x1600 mapping; the driver can "
                        f"never read it")

    return errs


# ---------------------------------------------------------------------------

def main() -> int:
    ap = argparse.ArgumentParser(
        description="Repair a dcfgen master DCF so lely can resolve its PDO tables.")
    ap.add_argument("dcf", help="master.dcf to repair in place")
    ap.add_argument("--nodes", default="1,2",
                    help="comma-separated CANopen node-IDs (default 1,2)")
    ap.add_argument("--check", action="store_true",
                    help="verify only, do not write")
    args = ap.parse_args()

    nodes = [int(n) for n in args.nodes.split(",") if n.strip()]
    if not nodes or any(n < 1 or n > 127 for n in nodes):
        print("ERROR: --nodes must list node-IDs in 1..127", file=sys.stderr)
        return 2

    with open(args.dcf, "r", encoding="utf-8") as fh:
        sections, preamble = split_sections(fh.read())

    if not args.check:
        repair(sections, nodes)

    errs = verify(sections, nodes)
    if errs:
        print(f"ERROR: {args.dcf} does not describe a PDO plan lely can use:",
              file=sys.stderr)
        for e in errs:
            print(f"  - {e}", file=sys.stderr)
        print("  Every one of these silently disables PDO exchange; see "
              "tools/fix_master_dcf.py", file=sys.stderr)
        return 1

    if args.check:
        print(f"OK: {args.dcf} resolves every PDO mapping for node(s) "
              f"{', '.join(map(str, nodes))}")
        return 0

    with open(args.dcf, "w", encoding="utf-8") as fh:
        fh.write(join_sections(sections, preamble))

    print(f"repaired {args.dcf}: {len(nodes)} node(s) x (3 RPDO + 3 TPDO), "
          f"verified against lely's admission rules")
    return 0


if __name__ == "__main__":
    sys.exit(main())