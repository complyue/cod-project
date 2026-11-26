# Facing Complexity with Scoped Effect Composition

*CoD developer guide · initial draft*

A factory wants to try a candidate detector on the same frame as its production detector. Production keeps control of the equipment. The candidate writes down what it would reject. Start with one part, a simulated camera and two recording outputs. The camera and the two outputs each keep a count. Before reading the wiring, predict how many acquisitions, production commands and shadow entries you expect.

## Lexical Names, Dynamic Bindings, and Static Checks

Publish the operation the detector is entitled to request in a shared vocabulary module. Give it a domain meaning precise enough for another team to implement.

```cod
# inspection-vocabulary.cod
symbol @request_reject:proc(part:u64) -> void
export proc @request_reject
```

The detector selects that identity from the vocabulary. It can now ask the active deployment what to do when a part crosses its threshold.

```cod
# detector.cod
symbol from "./inspection-vocabulary.cod" @request_reject

export proc inspect(part:u64, sample:u64, threshold:u64) -> void {
    if (sample >= threshold) { behave @request_reject(part) }
}
```

A separately declared symbol of the same name would have a distinct identity. Selecting the vocabulary supplies the contract. A reachable installation supplies the handler. For a **procedure symbol**, choose the call form at the site that knows which interpretation it needs.

| Call | What happens |
| --- | --- |
| `@op(x)` | Call the lexically, statically bound procedure family. Ordinary method dispatch applies within it. A replacement handler installed for that symbol does not intercept this call. |
| `behave @op(x)` | Call the currently installed handler using the dynamic bindings effective at invocation. |
| `perform @op(x)` | Call the currently installed handler and replay its required bindings captured **at that installation**. Installing the same definition again can capture different bindings. |

[Shared vocabulary identities](../design/ModuleWorlds.md#vocabulary-modules-establish-shared-identities) · [Procedure call and installation rules](../design/SymbolicDynamicScope.md#lexical-selection-installation-and-invocation)

## Wire production and shadow to one acquisition

In the application module, select `inspect` and the same vocabulary symbol. Use `check_shadow()` as the exercise entry. The fixture camera returns the scalar sample 80 for part 42. Its acquisition counter advances inside the camera. Use ordinary thresholds 90 for production and 70 for the candidate. Both outputs below are recording adapters, so the exercise needs no equipment.

```cod
using "./detector.cod" inspect
symbol from "./inspection-vocabulary.cod" @request_reject

proc acquire(count:u64 in O) -> u64 where {const owner O} {
    count <- count + 1u64
    return 80u64
}

proc check_shadow() -> void {
    val acquisitions <- 0u64
    val commands <- 0u64
    val would_reject <- 0u64
    val sample <- acquire(acquisitions)

    effect @request_reject(part:u64) -> void {
        assert part == 42u64
        commands <- commands + 1u64
    }
    inspect(42u64, sample, 90u64)
    {{
        effect @request_reject(part:u64) -> void {
            assert part == 42u64
            would_reject <- would_reject + 1u64
        }
        inspect(42u64, sample, 70u64)
    }}
    assert acquisitions == 1u64
    assert commands == 0u64
    assert would_reject == 1u64
}
```

Both runs use the same sample. The candidate issues the only request. Its inner installation interprets that request as a shadow observation. Leaving the scope restores the production installation. As a control, remove only the inner installation. The counters become **1 / 1 / 0**. Every request still has a compatible handler, so detecting the wrong business policy requires checking the destination. Now try invoking the detector from an entry with **no** installation on its calling path. The checking contract requires rejection for the unsupplied operation. Pre-2.0 checking takes the union of outstanding requirements over the selected procedure family. [Binding availability and checker scope](../design/SymbolicDynamicScope.md#family-wide-effect-requirements-before-20)

## Diagnose an unexpected destination

Use the [worked scoped audit](../papers/examples/ScopedTileAudit.md#contract-and-observations). Its recorder installs at destination **1**. The caller then selects **9** and the tile temporarily selects **2**. Predict the audit before reading the check. With `perform`, the recorder writes at 1. With `behave`, it writes at 2. After recording, the tile sees 2. After the tile returns or throws its checked fault, the caller sees 9. The completed audit entry remains visible. When debugging a shadow leak, identify which handler was installed, which bindings that installation captured, and which verb the caller used. Capturing a reference preserves its actual-owner dependency. [Capture and owner lifetime](../design/SymbolicDynamicScope.md#regional-resource-capture-preserves-owners)

## Add an audit without editing the detectors

Keep `@request_reject` as the vocabulary operation for a proposal. Declare a separate `@reject_command_issued` audit operation at the device adapter for “reject command issued.” To add a request audit to this fixture, introduce `val requests <- 0u64` beside the other counters. Define an ordinary helper that increments a supplied audit counter, and call it at the start of both installed request handlers. Leave `inspect` alone. After the two runs, assert `requests == 1u64`, `commands == 0u64` and `would_reject == 1u64`. Replacing the counters with recorders keyed by part gives one request for part 42 and no issued-command record. A second detector using the same request protocol participates immediately. A detector that calls the driver directly bypasses that boundary. An audit promising coverage of all device commands needs control over the driver entry points.

When audit and retry must wrap the same work, pass a bounded `here` body or an explicit next operation and specify who invokes it. Re-entering `behave @op` from its own replacement selects the active handler again. `perform` replays the required bindings captured when that handler was installed. The next step in a wrapper must be supplied explicitly. An outer audit can count one logical inspection while an audit inside retry counts two attempts. Choose the observation the business requested.

As a next exercise, let a new detector request a manual inspection. Decide whether a shadow run may open a real work order, then supply that policy where the run is assembled. The [sister paper](../papers/MarginalCostOfComposition.md) asks what this change should cost each author. The examples here illustrate source contracts and expected observations. For a production exercise, record which frontend and revision accept the module forms and execute the assertions. Executable packaging, a longer troubleshooting walkthrough and measured onboarding results remain future work.

