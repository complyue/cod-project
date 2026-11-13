"""Conservative implicit-context summaries for explicit fixture contracts.

This checks the dependency model, not inferred effects of arbitrary C++/CoD.
A production checker would establish these contracts from checked semantics.
"""

from dataclasses import dataclass, field


@dataclass(frozen=True)
class Contract:
    reads: frozenset[str] = field(default_factory=frozenset)
    installs: frozenset[str] = field(default_factory=frozenset)
    touches: frozenset[str] = field(default_factory=frozenset)
    calls: tuple[str, ...] = ()
    deferred: tuple[str, ...] = ()


def summarize(contracts):
    fields = ("reads", "installs", "touches")
    state = {name: {f: set(getattr(c, f)) for f in fields} for name, c in contracts.items()}
    changed = True
    while changed:
        changed = False
        for name, c in contracts.items():
            for target in c.calls + c.deferred:
                if target not in contracts:
                    raise ValueError(f"missing context contract for {target}")
                for f in fields:
                    merged = state[name][f] | state[target][f]
                    if merged != state[name][f]:
                        state[name][f] = merged
                        changed = True
    return {name: dict({f: sorted(v) for f, v in s.items()},
                       required=sorted(set().union(*s.values()))) for name, s in state.items()}


def observations():
    contracts = {
        "evaluate_expr": Contract(calls=("evaluate_expr",)),
        "lower_from_current": Contract(reads=frozenset(("lower",)), calls=("evaluate_expr",)),
        "touch_only": Contract(touches=frozenset(("lower",))),
        "deferred_diagnostic": Contract(reads=frozenset(("diagnostic",))),
        "opaque_rebind": Contract(installs=frozenset(("lower",))),
        "compile_adapter": Contract(reads=frozenset(("expression",)), calls=("lower_from_current",)),
        "cleanup_scope": Contract(calls=("touch_only", "opaque_rebind"), deferred=("deferred_diagnostic",)),
    }
    result = summarize(contracts)
    assert result["evaluate_expr"]["required"] == []
    assert result["touch_only"]["reads"] == [] and result["touch_only"]["required"] == ["lower"]
    assert result["compile_adapter"]["required"] == ["expression", "lower"]
    assert result["cleanup_scope"]["required"] == ["diagnostic", "lower"]
    assert result["cleanup_scope"]["installs"] == ["lower"]
    required = set(result["compile_adapter"]["required"])
    captured = {"lower"}
    ambient_for_perform = sorted(required - captured)
    assert ambient_for_perform == ["expression"]
    try:
        summarize({"caller": Contract(calls=("unmodelled_native",))})
    except ValueError as e:
        rejection = str(e)
    else:
        raise AssertionError("unknown call must not be treated as context-free")
    return {"role": "dependency-model observation using explicit contracts; no automatic source effect inference",
            "summaries": result, "perform_ambient_projection": ambient_for_perform,
            "perform_captured_projection": sorted(captured), "unknown_call": rejection,
            "abi": "unspecified; explicit fixture arguments and TLS adapters are experimental"}
