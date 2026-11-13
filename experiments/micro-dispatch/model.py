"""Finite named CoD-type fixture and explicit stable definition-owned identity mapping.

No source parser, implicit generic instantiator or production identity ABI.
"""
from dataclasses import dataclass
import json


@dataclass(frozen=True)
class Type:
    name: str
    abstract: bool = False
    parents: tuple[str, ...] = ()
    size: int = 0
    align: int = 1
    # (field name, byte offset, storage kind, declared alternatives)
    fields: tuple[tuple, ...] = ()


def word(name, parents):
    return Type(name, parents=parents, size=16, align=8,
                fields=(("value", 0, "i64", ()), ("site", 8, "i64", ())))


TYPES = [Type("Datum", True), Type("Plan", True, ("Datum",)),
         Type("LiteralPlan", True, ("Plan",)), Type("SourceLocated", True, ("Datum",)),
         Type("Scalar", True, ("Datum",)), Type("Quantity", True, ("Scalar",)),
         Type("Destination", True, ("Datum",)), Type("AddressDestination", True, ("Destination",)),
         Type("Summary", True, ("Datum",)),
         word("IntegerLiteral", ("Scalar", "SourceLocated")), word("BooleanLiteral", ("Scalar", "SourceLocated")),
         word("Meters", ("Quantity",)), word("Seconds", ("Quantity",)),
         word("LocalSlot", ("Destination",)),
         Type("RegionalSlot", parents=("AddressDestination",), size=32, align=16,
              fields=(("region_symbol", 0, "i64", ()), ("value", 8, "i64", ()), ("site", 16, "i64", ()))),
         Type("Example", parents=("LiteralPlan", "SourceLocated"), size=40, align=8,
              fields=(("x", 0, "choice", ("IntegerLiteral", "BooleanLiteral")),
                      ("y", 16, "choice", ("LocalSlot", "RegionalSlot")), ("site", 32, "i64", ()))),
         Type("Other", parents=("Plan", "SourceLocated"), size=40, align=8,
              fields=(("site", 0, "i64", ()), ("y", 8, "choice", ("LocalSlot", "RegionalSlot")),
                      ("x", 24, "choice", ("IntegerLiteral", "BooleanLiteral")))),
         word("ValueSummary", ("Summary",)), word("AddressSummary", ("Summary",))]


# Definition-owned experimental IDs. These literal assignments are durable
# fixture data, never derived from inventory position, discovery order or a hash.
# The semantic-key catalog detects conflicting assignments across readers.
STABLE_IDS = {
    "Datum": 0xC0D0000100000001, "Plan": 0xC0D0000100000002,
    "LiteralPlan": 0xC0D0000100000003, "SourceLocated": 0xC0D0000100000004,
    "Scalar": 0xC0D0000100000005, "Quantity": 0xC0D0000100000006,
    "Destination": 0xC0D0000100000007, "AddressDestination": 0xC0D0000100000008,
    "Summary": 0xC0D0000100000009,
    "IntegerLiteral": 0xC0D0000100000011, "BooleanLiteral": 0xC0D0000100000012,
    "Meters": 0xC0D0000100000013, "Seconds": 0xC0D0000100000014,
    "LocalSlot": 0xC0D0000100000015, "RegionalSlot": 0xC0D0000100000016,
    "Example": 0xC0D0000100000017, "Other": 0xC0D0000100000018,
    "ValueSummary": 0xC0D0000100000019, "AddressSummary": 0xC0D000010000001A,
    "EarlierUnrelated": 0xC0D000010000FF00,
    "Node": 0xC0D0000200000001, "Leaf": 0xC0D0000200000002,
    "AUnrelated": 0xC0D000020000FF00,
}


class ModelError(ValueError):
    pass


class Universe:
    def __init__(self, types=TYPES, codes=None):
        self.types = {t.name: t for t in types}
        if len(self.types) != len(types):
            raise ModelError("duplicate semantic definition")
        for t in types:
            if t.abstract and len(t.parents) > 1:
                raise ModelError("multiple abstract parents")
            self.ancestors(t.name)
        if codes is None and any(name not in STABLE_IDS for name in self.types):
            raise ModelError("explicit stable definition ID required")
        self.type_ids = ({name: STABLE_IDS[name] for name in self.types} if codes is None else codes)
        if set(self.type_ids) != set(self.types) or any(not 0 < value < 2**64 for value in self.type_ids.values()):
            raise ModelError("invalid concrete/abstract identity mapping")
        if len(set(self.type_ids.values())) != len(self.type_ids):
            raise ModelError("ambiguous discriminator mapping")
        # Traverse references even though runtime discriminators need no checksum.
        for name in self.types:
            self.canonical(name)

    def ancestors(self, name, active=()):
        if name not in self.types:
            raise ModelError(f"unresolved type/generic instance: {name}")
        if name in active:
            raise ModelError("conformance cycle")
        return {name}.union(*(self.ancestors(p, active + (name,)) for p in self.types[name].parents))

    def subtype(self, left, right):
        return right in self.ancestors(left)

    def members(self, constraint):
        return frozenset(name for name, t in self.types.items() if not t.abstract and self.subtype(name, constraint))

    def canonical(self, root):
        # Illustrative root-compatibility description; never a runtime dispatch key.
        # Named backreferences preserve cycles/sharing without recursive hashing.
        # Concrete conformance is a selected relation, not value identity.
        # Identity and definition revision belong to the selected semantic input;
        # this named-graph encoding is experimental, not structural graph isomorphism.
        pending, seen, nodes = [root], set(), []
        while pending:
            name = pending.pop()
            if name in seen:
                continue
            if name not in self.types:
                raise ModelError(f"unresolved type/generic instance: {name}")
            seen.add(name)
            t = self.types[name]
            fields = [(f, o, k, sorted(a)) for f, o, k, a in t.fields]
            nodes.append({"identity": name, "revision": 1, "abstract": t.abstract,
                          "parents": sorted(t.parents) if t.abstract else [], "size": t.size, "align": t.align, "fields": fields})
            pending.extend(t.parents if t.abstract else ())
            pending.extend(ref for _, _, _, alternatives in t.fields for ref in alternatives)
        return json.dumps({"encoding": "named-type-graph-v1", "target": "fixture-lp64-le",
                           "root": root, "nodes": sorted(nodes, key=lambda n: n["identity"])},
                          sort_keys=True, separators=(",", ":"))

    def layout(self, name):
        # Diagnostic grouping only. Local access groups by the exact operation's
        # offset/storage contract, so no parallel runtime representation tag is needed.
        t = self.types[name]
        return (t.size, t.align, tuple((offset, kind) for _, offset, kind, _ in t.fields))


@dataclass(frozen=True)
class Method:
    name: str
    signature: tuple[str, ...]
    bias: int


METHODS = [Method("general", ("Plan", "Destination"), 10),
           Method("literal", ("LiteralPlan", "Destination"), 20),
           Method("address", ("Plan", "AddressDestination"), 30),
           Method("intersection", ("LiteralPlan", "AddressDestination"), 40)]


def dominates(u, a, b):
    return a.signature != b.signature and all(u.subtype(x, y) for x, y in zip(a.signature, b.signature))


def intersect(a, b):
    out = tuple(x & y for x, y in zip(a, b))
    return out if all(out) else None


def subtract(rect, cut):
    overlap = intersect(rect, cut)
    if overlap is None:
        return [rect]
    out, prefix = [], []
    for i, (axis, inside) in enumerate(zip(rect, overlap)):
        if axis - inside:
            out.append(tuple(prefix + [axis - inside] + list(rect[i + 1:])))
        prefix.append(inside)
    return out


def coverage(u, parameters, methods, budget=512):
    """Finite symbolic rectangles, selectively adapted from archived families.py."""
    domain = tuple(u.members(p) for p in parameters)
    if not all(domain):
        raise ModelError("empty domain")
    extents = []
    for m in methods:
        if len(m.signature) != len(parameters) or not all(u.subtype(s, p) for s, p in zip(m.signature, parameters)):
            raise ModelError("invalid method signature")
        extents.append(tuple(u.members(s) for s in m.signature))
    steps = 0
    def residual(rect, cuts):
        nonlocal steps
        pieces = [rect]
        for cut in cuts:
            out = []
            for p in pieces:
                steps += 1
                if steps > budget:
                    raise ModelError("coverage budget exhausted; no proof")
                out.extend(subtract(p, cut))
            pieces = out
        return pieces
    if residual(domain, extents):
        raise ModelError("uncovered call domain")
    for i, left in enumerate(methods):
        for j, right in enumerate(methods[:i]):
            if dominates(u, left, right) or dominates(u, right, left):
                continue
            overlap = intersect(extents[i], extents[j])
            if overlap is None:
                continue
            rem = residual(overlap, [extents[k] for k, m in enumerate(methods)
                                    if dominates(u, m, left) or dominates(u, m, right)])
            if rem:
                raise ModelError(f"ambiguous {left.name}/{right.name}: {tuple(sorted(x)[0] for x in rem[0])}")
    return {"status": "proved for finite selected domain", "rectangle_steps": steps,
            "methods": [m.name for m in methods], "tuple_enumeration": False}


def callsite_targets(u, methods, possible):
    """Prove possible targets from checked call-site sets, without tuple expansion."""
    result=[]
    extents=[tuple(u.members(t) for t in m.signature) for m in methods]
    for i, method in enumerate(methods):
        overlap=intersect(tuple(map(frozenset,possible)),extents[i])
        if overlap is None: continue
        pieces=[overlap]
        for j, other in enumerate(methods):
            if dominates(u,other,method):
                pieces=[p for r in pieces for p in subtract(r,extents[j])]
        if pieces:result.append(method.name)
    return result
