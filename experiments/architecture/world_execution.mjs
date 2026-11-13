// Experimental execution boundary, not a CoD parser, checker, allocator or ABI.
import assert from 'node:assert/strict';
import { createHash } from 'node:crypto';
import { performance } from 'node:perf_hooks';
import { readFileSync } from 'node:fs';

const hash = value => createHash('sha256').update(typeof value === 'string' ? value : JSON.stringify(value)).digest('hex');
const mode = process.argv[2] ?? 'interpret';
const typeCount = Number(process.argv[3] ?? 8);
const viewCount = Number(process.argv[4] ?? 4);
const iterations = Number(process.argv[5] ?? 20000);
assert(['interpret', 'prepared', 'tiered'].includes(mode));
assert(typeCount >= 3 && viewCount >= 3);

// Second-argument classes are fixture-specific disjoint sets of semantic type IDs,
// not layout equivalence or a proposed language conversion/subtyping rule.
// Complete layout for this finite, nonrecursive experimental record. The
// self-relative reference is a lower-level scalar, not a recursive hash policy.
const layout = {
  target: 'experimental-wasm32-le', size: 16, alignment: 8,
  fields: [{ offset: 0, type: { bits: 32, signed: true } },
    { offset: 8, type: { bits: 64, signed: true, reference: 'self-relative' } }],
  padding: [[4, 8]],
};
const layoutId = hash(layout);
const types = Array.from({ length: typeCount }, (_, id) => ({ id, semantic: `domain/type-${id}`, dispatchClass: id % 3, layout, layoutId }));
const sumLayout = { size: 24, alignment: 8, tag: { offset: 0, bits: 32 },
  alternatives: types.map(t => ({ tag: t.id, offset: 8, layout: t.layout })) };
// Experimental SHA-256 and JSON order are reproducibility choices, not a
// proposed canonicalization, recursive encoding or persistence format.
const views = Array.from({ length: viewCount }, (_, id) => ({
  id, origin: id === 0 ? 'entry' : id === 1 ? 'dependency-local' : `source-${id}`,
  bias: id * 1000,
  methods: types.flatMap(t => [0, 1, 2].map(secondClass => ({
    firstType: t.id, secondClass, kernel: (t.id + secondClass + 2) % 3, semanticBias: t.id * 10,
  }))),
}));
const artifact = { types, sumLayout, views, entry: 0, entryExtends: true, entryInstalls: true,
  reachableProcedures: [{ name: 'dependency.helper', installsView: 1 }],
  sourceObservedPairs: [[0, 0]],
  externalInput: { carrierType: 'experiment/graph-envelope', describedTypes: types.map(t => t.semantic) },
};
// Only a finite-model structural check. General CoD inference, ownership proofs,
// generics and external definition admission remain outside this experiment.
function checkArtifact(a) {
  assert(!a.entryExtends || a.entryInstalls, 'entry extension requires installation');
  assert.equal(new Set(a.types.map(t => t.semantic)).size, typeCount);
  for (const p of a.reachableProcedures) assert(a.views.some(v => v.id === p.installsView), 'missing dependency interpretation');
  for (const v of a.views) for (const t of a.types) for (let secondClass = 0; secondClass < 3; ++secondClass) {
    const selected = v.methods.filter(m => m.firstType === t.id && m.secondClass === secondClass);
    assert.equal(selected.length, 1, 'missing or ambiguous method');
    assert([0, 1, 2].includes(selected[0].kernel));
  }
}
const checkStart = performance.now();
checkArtifact(artifact);
const checkingMs = performance.now() - checkStart;
assert.throws(() => checkArtifact({ ...artifact, views: [artifact.views[0]] }), /missing dependency/);
assert.throws(() => checkArtifact({ ...artifact, entryInstalls: false }), /entry extension/);
assert.equal(types[0].layoutId, types[1].layoutId);
assert.notEqual(types[0].semantic, types[1].semantic);

class Regions {
  constructor() {
    this.memory = new WebAssembly.Memory({ initial: 2 });
    this.data = new DataView(this.memory.buffer);
    this.owners = new Map();
    this.nextBase = 64;
    this.acquired = 0;
    this.released = 0;
  }
  acquire(values, type = 0) {
    const owner = this.acquired++;
    const base = this.nextBase; this.nextBase += 128;
    this.owners.set(owner, { base, alive: true, size: values.length * 16 });
    for (let i = 0; i < values.length; ++i) {
      this.data.setInt32(base + i * 16, values[i], true);
      // Each link field points to the first payload relative to its own address.
      this.data.setBigInt64(base + i * 16 + 8, BigInt(-i * 16 - 8), true);
    }
    return { owner, offset: 0, type, readonly: false };
  }
  address(ref) {
    const region = this.owners.get(ref.owner);
    if (!region?.alive) throw Error('expired owner');
    assert(ref.offset >= 0 && ref.offset + 16 <= region.size);
    return region.base + ref.offset;
  }
  read(ref) { return this.data.getInt32(this.address(ref), true); }
  write(ref, value) {
    if (ref.readonly) throw Error('readonly reference');
    this.data.setInt32(this.address(ref), value, true);
  }
  remap(ref) {
    const region = this.owners.get(ref.owner);
    const oldBase = this.address({ ...ref, offset: 0 });
    const bytes = new Uint8Array(this.memory.buffer, oldBase, region.size).slice();
    const newBase = this.nextBase; this.nextBase += 128;
    new Uint8Array(this.memory.buffer, newBase, bytes.length).set(bytes);
    region.base = newBase;
    assert.deepEqual(new Uint8Array(this.memory.buffer, newBase, bytes.length), bytes);
    const field = this.address(ref) + 8;
    const target = field + Number(this.data.getBigInt64(field, true));
    assert.equal(target, newBase);
    return { payloadBytes: bytes.length, patchedFields: 0, baseChanged: oldBase !== newBase };
  }
  release(ref) {
    const region = this.owners.get(ref.owner);
    assert(region.alive); region.alive = false; ++this.released;
  }
}

// Three shared method kernels read the same linear memory as the interpreter.
// The caller supplies checked temporary addresses and semantic/context bias.
// No kernel embeds a regional address, owner identity or TLS value.
const operations = [
  ['left', 'right', 'add', 'bias', 'add'],
  ['left', 'right', 'sub', 'bias', 'add'],
  ['left', 'right', 'mul', 'bias', 'add'],
];
const uleb = n => { const b = []; do { const v = n & 127; n >>>= 7; b.push(v | (n ? 128 : 0)); } while (n); return b; };
const str = s => [...uleb(Buffer.byteLength(s)), ...Buffer.from(s)];
const vector = rows => [...uleb(rows.length), ...rows.flat()];
const section = (id, bytes) => [id, ...uleb(bytes.length), ...bytes];
function wasmBytes(kind) {
  const opcodes = { left: [0x20, 0, 0x28, 2, 0], right: [0x20, 1, 0x28, 2, 0],
    bias: [0x20, 2], add: [0x6a], sub: [0x6b], mul: [0x6c] };
  const code = [0, ...operations[kind].flatMap(op => opcodes[op]), 0x0b];
  return new Uint8Array([0, 97, 115, 109, 1, 0, 0, 0,
    ...section(1, vector([[0x60, 3, 0x7f, 0x7f, 0x7f, 1, 0x7f]])),
    ...section(2, vector([[...str('env'), ...str('memory'), 2, 0, 2]])),
    ...section(3, vector([[0]])),
    ...section(7, vector([[...str('invoke'), 0, 0]])),
    ...section(10, vector([[...uleb(code.length), ...code]])),
  ]);
}
class Engine {
  constructor(mode, regions) {
    this.mode = mode; this.regions = regions; this.native = new Map();
    this.counts = [0, 0, 0]; this.bytes = 0; this.compileMs = 0;
    this.interpretedCalls = 0; this.nativeCalls = 0;
    if (mode === 'prepared') for (let kind = 0; kind < 3; ++kind) this.compile(kind);
    this.preparedKernels = this.native.size;
  }
  compile(kind) {
    const bytes = wasmBytes(kind); const start = performance.now();
    const module = new WebAssembly.Module(bytes);
    const instance = new WebAssembly.Instance(module, { env: { memory: this.regions.memory } });
    this.compileMs += performance.now() - start; this.bytes += bytes.length;
    this.native.set(kind, instance.exports.invoke);
  }
  invoke(kind, left, right, bias) {
    ++this.counts[kind];
    // A local experimental trigger, not a proposed language/runtime threshold.
    if (this.mode === 'tiered' && this.counts[kind] === 8) this.compile(kind);
    const native = this.native.get(kind);
    if (native) { ++this.nativeCalls; return native(left, right, bias); }
    ++this.interpretedCalls;
    const stack = [];
    for (const op of operations[kind]) {
      if (op === 'left') stack.push(this.regions.data.getInt32(left, true));
      else if (op === 'right') stack.push(this.regions.data.getInt32(right, true));
      else if (op === 'bias') stack.push(bias);
      else {
        const b = stack.pop(), a = stack.pop();
        stack.push(op === 'add' ? (a + b) | 0 : op === 'sub' ? (a - b) | 0 : Math.imul(a, b));
      }
    }
    return stack.pop();
  }
}
const copy = value => structuredClone(value);
class Execution {
  constructor(engine, regions) {
    this.engine = engine; this.regions = regions;
    this.current = { bias: 7, ref: null, callable: null };
    this.events = []; this.touches = 0;
  }
  capture(view, ref) { ++this.touches; return { view, captures: { bias: this.current.bias, ref: copy(ref) } }; }
  invoke(callable, left, right, captured) {
    const view = artifact.views.find(v => v.id === callable.view);
    assert(view, 'unfused view');
    const secondType = artifact.types[right.type];
    const method = view.methods.find(m => m.firstType === left.type && m.secondClass === secondType?.dispatchClass);
    assert(method && artifact.types[right.type], 'unknown semantic type');
    const context = captured ? callable.captures : this.current;
    const bias = view.bias + method.semanticBias + right.type + context.bias + this.regions.read(context.ref);
    return this.engine.invoke(method.kernel, this.regions.address(left), this.regions.address(right), bias);
  }
  install(view, ref, { suppress = false, fail = false } = {}) {
    this.events.push(`operand:${view}`);
    const value = this.capture(view, ref);
    try {
      if (fail) throw Error('operand failed');
      if (!suppress) { this.current.callable = value; this.events.push(`write:${view}`); }
    } finally { this.events.push(`operand-cleanup:${view}`); }
  }
  dependencyLocal(ref, action) {
    const saved = copy(this.current);
    try { this.install(1, ref); return action(); }
    finally { this.current = saved; this.events.push('restore:dependency'); }
  }
}
// Completion machine is shared test scaffolding, not native exception unwinding.
function unwind(pending, actions, trace) {
  for (const action of [...actions].reverse()) {
    const previousReturn = pending.kind === 'return' ? pending : pending.savedReturn;
    try {
      let next;
      if (pending.kind === 'throw') next = action.catch?.(pending.error);
      else next = action.body?.();
      if (next) pending = next;
      else if (pending.kind === 'throw' && action.catch) pending = previousReturn ?? { kind: 'normal' };
    } catch (error) { pending = { kind: 'throw', error: error.message, savedReturn: previousReturn }; }
    finally {
      trace.push(`finally:${action.name}`);
      try { const next = action.finally?.(); if (next) pending = next; }
      catch (error) { pending = { kind: 'throw', error: error.message, savedReturn: previousReturn }; }
    }
  }
  return pending;
}

const start = performance.now();
const regions = new Regions();
const owner = regions.acquire([11, 13]);
const other = regions.acquire([5]);
const left = { ...owner, type: 0 }, right = { ...owner, offset: 16, type: 1 };
const engine = new Engine(mode, regions); const execution = new Execution(engine, regions);
execution.current.ref = copy(other);
const first = execution.capture(0, other); const independent = copy(first);
first.captures.bias = 70; assert.equal(independent.captures.bias, 7);
assert.equal(execution.touches, 1); assert.equal(engine.interpretedCalls + engine.nativeCalls, 0);
execution.install(1, other, { suppress: true }); assert.equal(execution.current.callable, null);
assert.deepEqual(execution.events, ['operand:1', 'operand-cleanup:1']);
execution.install(0, other);
const installed = copy(execution.current.callable);
assert.throws(() => execution.install(1, other, { suppress: true, fail: true }), /operand failed/);
assert.deepEqual(execution.current.callable, installed);
execution.current.bias = 19;
const observations = {
  captured: execution.invoke(independent, left, right, true),
  current: execution.invoke(independent, left, right, false),
  dependency: execution.dependencyLocal(other, () => execution.invoke(execution.current.callable, left, right, false)),
  source: execution.invoke(execution.capture(2, other), left, right, true),
  sameLayoutDifferentType: execution.invoke(independent, { ...left, type: 1 }, right, true),
};
assert.deepEqual(observations, { captured: 37, current: 49, dependency: 1049, source: 2049, sameLayoutDifferentType: 21 });
execution.current.ref = copy(owner);
assert.equal(execution.invoke(independent, left, right, false), 55);
assert.equal(execution.invoke(independent, left, right, true), 37);
execution.current.ref = copy(other);
const relocation = regions.remap(owner);
assert.equal(execution.invoke(independent, left, right, true), observations.captured);
assert.equal(independent.captures.ref.owner, other.owner);
assert.throws(() => regions.write({ ...left, readonly: true }, 4), /readonly/);
// External carrier is supplied after artifact construction. Its described pair
// was not in sourceObservedPairs, but both definitions are in this fixture's
// artifact. Admission of new definitions is deliberately not modelled.
const envelope = { carrier: artifact.externalInput.carrierType, pair: [typeCount - 1, typeCount - 2] };
assert(!artifact.sourceObservedPairs.some(p => p[0] === envelope.pair[0] && p[1] === envelope.pair[1]));
const externalResult = execution.invoke(independent, { ...left, type: envelope.pair[0] }, { ...right, type: envelope.pair[1] }, true);
const observedOnly = new Set(artifact.sourceObservedPairs.map(p => p.join(',')));
assert(!observedOnly.has(envelope.pair.join(',')), 'observed-only control must miss this input');
const trace = [];
let pending = unwind({ kind: 'return', value: 42 }, [
  { name: 'catch', catch: error => { assert.equal(error, 'cleanup failed'); trace.push('caught'); } },
  { name: 'throw', body: () => { throw Error('cleanup failed'); } },
], trace);
assert.deepEqual(pending, { kind: 'return', value: 42 });
assert.deepEqual(trace, ['finally:throw', 'caught', 'finally:catch']);
const replacementTrace = [];
pending = unwind({ kind: 'throw', error: 'body failed' }, [
  { name: 'last', body: () => { replacementTrace.push('remaining'); } },
  { name: 'recover', catch: () => ({ kind: 'return', value: 8 }), finally: () => ({ kind: 'return', value: 9 }) },
], replacementTrace);
assert.deepEqual(pending, { kind: 'return', value: 9 });
assert.deepEqual(replacementTrace, ['finally:recover', 'remaining', 'finally:last']);
const coldMs = performance.now() - start;
// Rebinding and type/view variation prevent freezing dynamic inputs. The
// engine boundary is measured, including dispatch, live-owner checks and JS
// calls. It is not a measurement of just native arithmetic throughput.
let checksum = 0; const batch = performance.now();
for (let i = 0; i < iterations; ++i) {
  execution.current.bias = i % 23;
  const callable = { ...independent, view: i % viewCount };
  checksum = (checksum + execution.invoke(callable, { ...left, type: i % typeCount },
    { ...right, type: (i * 7 + 1) % typeCount }, i % 2 === 0)) | 0;
}
const executionMs = performance.now() - batch;
regions.release(other);
assert.throws(() => execution.invoke(independent, left, right, true), /expired owner/);
regions.release(owner);
assert.equal(regions.acquired, regions.released);
console.log(JSON.stringify({ schema: 1, mode, typeCount, viewCount, iterations,
  node: process.version, v8: process.versions.v8, flags: process.execArgv,
  sourceSha256: hash(readFileSync(new URL(import.meta.url), 'utf8')),
  artifactSha256: hash(artifact), artifactBytes: Buffer.byteLength(JSON.stringify(artifact)),
  semanticRecords: types.length + views.reduce((n, v) => n + 1 + v.methods.length, 0),
  tupleProduct: typeCount ** 2 * viewCount, layoutId, sumLayoutId: hash(sumLayout),
  checkingMs, coldMs, executionMs, compileMs: engine.compileMs,
  preparedKernels: engine.preparedKernels, nativeKernels: engine.native.size,
  nativeModuleBytes: engine.bytes, nativeCalls: engine.nativeCalls, interpretedCalls: engine.interpretedCalls,
  observations, externalResult, checksum, relocation, regions: { acquired: regions.acquired, released: regions.released },
  trace, replacementTrace, events: execution.events,
  checks: ['entry extension requires installation', 'entry-only artifact rejected', 'same-layout semantic distinction', 'unseen input pair',
    'observed-only control misses input', 'captured/current binding', 'dependency local installation',
    'source interpretation', 'suppressed write preserves operands/failure/cleanup', 'independent copy',
    'touch does not execute', 'unchanged relative payload on remap', 'readonly/expired owner rejection',
    'pending return recovery', 'cleanup return replacement and remaining actions'],
}));
