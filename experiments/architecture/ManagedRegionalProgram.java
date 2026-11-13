// Isolated managed-host adapter. No production representation or ABI is selected.
import java.lang.foreign.*;
import java.lang.invoke.MethodHandle;
import java.lang.management.ManagementFactory;
import java.nio.file.*;
import java.util.*;
import static java.lang.foreign.ValueLayout.*;

public final class ManagedRegionalProgram {
    static void require(boolean condition, String message) {
        if (!condition) throw new AssertionError(message);
    }
    record Event(long tag, long a, long b) {}
    record Case(String name, long[] arguments, long[] result, List<Event> events) {}
    static final class API {
        final Linker linker = Linker.nativeLinker();
        final SymbolLookup symbols;
        final MethodHandle open, close, base, size, layout, directory, field, environment, policy, command, nativeRun, events, finish;
        long crossings;
        API(Path library, Arena arena) {
            symbols = SymbolLookup.libraryLookup(library, arena);
            open = function("bridge_open", FunctionDescriptor.of(ADDRESS, ADDRESS));
            close = function("bridge_close", FunctionDescriptor.ofVoid(ADDRESS));
            base = function("bridge_base", FunctionDescriptor.of(ADDRESS, ADDRESS, JAVA_LONG));
            size = function("bridge_size", FunctionDescriptor.of(JAVA_LONG, ADDRESS));
            layout = function("bridge_layout", FunctionDescriptor.ofVoid(ADDRESS));
            directory = function("bridge_directory", FunctionDescriptor.ofVoid(ADDRESS, JAVA_LONG, ADDRESS));
            field = function("bridge_field", FunctionDescriptor.of(JAVA_LONG, ADDRESS, JAVA_LONG, JAVA_LONG, JAVA_LONG, JAVA_LONG));
            environment = function("bridge_environment", FunctionDescriptor.of(ADDRESS, JAVA_LONG, JAVA_LONG, JAVA_LONG, JAVA_LONG));
            policy = function("bridge_policy_address", FunctionDescriptor.of(ADDRESS, ADDRESS, JAVA_LONG));
            command = function("bridge_command", FunctionDescriptor.of(JAVA_LONG, ADDRESS, JAVA_LONG, JAVA_LONG));
            nativeRun = function("bridge_native", FunctionDescriptor.ofVoid(ADDRESS, ADDRESS, ADDRESS));
            events = function("bridge_events", FunctionDescriptor.of(JAVA_LONG, ADDRESS, ADDRESS, JAVA_LONG));
            finish = function("bridge_finish", FunctionDescriptor.ofVoid(ADDRESS, ADDRESS));
        }
        MethodHandle function(String name, FunctionDescriptor signature) {
            return linker.downcallHandle(symbols.find(name).orElseThrow(), signature);
        }
        long command(MemorySegment environment, long operation, long argument) throws Throwable {
            ++crossings; return (long) command.invokeExact(environment, operation, argument);
        }
    }
    static final class Graph implements AutoCloseable {
        final API api;
        final Arena lifetime = Arena.ofConfined();
        final MemorySegment handle;
        final MemorySegment[] mappings = new MemorySegment[2];
        final long[] layout, nodes, procedures;
        final byte[] original;
        Graph(API api, Path path) throws Throwable {
            this.api = api;
            handle = (MemorySegment) api.open.invokeExact(lifetime.allocateUtf8String(path.toString()));
            require(!handle.equals(MemorySegment.NULL), "native fixture construction failed");
            long size = (long) api.size.invokeExact(handle);
            for (int i = 0; i < 2; ++i) {
                MemorySegment pointer = (MemorySegment) api.base.invokeExact(handle, (long) i);
                mappings[i] = pointer.reinterpret(size, lifetime, null).asReadOnly();
            }
            require(mappings[0].address() != mappings[1].address(), "mappings share an address");
            original = mappings[0].toArray(JAVA_BYTE);
            require(Arrays.equals(original, mappings[1].toArray(JAVA_BYTE)), "mapped bytes differ");
            MemorySegment metadata = lifetime.allocate(18 * 8, 8);
            api.layout.invokeExact(metadata); layout = metadata.toArray(JAVA_LONG);
            MemorySegment nodeOffsets = lifetime.allocate(layout[12] * 8, 8);
            MemorySegment procedureOffsets = lifetime.allocate(layout[13] * 8, 8);
            api.directory.invokeExact(handle, 0L, nodeOffsets); nodes = nodeOffsets.toArray(JAVA_LONG);
            api.directory.invokeExact(handle, 1L, procedureOffsets); procedures = procedureOffsets.toArray(JAVA_LONG);
            try { mappings[0].set(JAVA_BYTE, 0, (byte) 0); throw new AssertionError("readonly write accepted"); }
            catch (UnsupportedOperationException expected) {}
            try { mappings[0].get(JAVA_LONG, size); throw new AssertionError("out-of-bounds read accepted"); }
            catch (IndexOutOfBoundsException expected) {}
        }
        long direct(int mapping, boolean procedure, int id, int field) {
            MemorySegment memory = mappings[mapping];
            long location;
            boolean relative;
            if (procedure) {
                location = procedures[id] + (field < 3 ? layout[8 + field] : layout[11] + (field - 3) * 4);
                relative = field == 0;
            } else {
                location = nodes[id] + layout[field]; relative = field >= 3 && field <= 6;
                if (field == 1) return memory.get(JAVA_LONG, location);
            }
            if (!relative) return Integer.toUnsignedLong(memory.get(JAVA_INT, location));
            long displacement = memory.get(JAVA_LONG, location);
            return displacement == 0 ? 0 : Integer.toUnsignedLong(memory.get(JAVA_INT, location + displacement + layout[7]));
        }
        @Override public void close() {
            require(Arrays.equals(original, mappings[0].toArray(JAVA_BYTE)), "program mutated");
            require(Arrays.equals(original, mappings[1].toArray(JAVA_BYTE)), "remapped program mutated");
            lifetime.close();
            try { mappings[0].get(JAVA_BYTE, 0); throw new AssertionError("closed program accepted"); }
            catch (IllegalStateException expected) {}
            try { api.close.invokeExact(handle); } catch (Throwable e) { throw new RuntimeException(e); }
        }
    }
    static final class Environment implements AutoCloseable {
        final API api;
        final MemorySegment handle;
        final Arena scratch = Arena.ofConfined();
        final Arena[] owners = {Arena.ofConfined(), Arena.ofConfined(), Arena.ofConfined()};
        final MemorySegment[] policies = new MemorySegment[3];
        long entries, frees;
        Environment(API api, long[] args) throws Throwable {
            this.api = api;
            handle = (MemorySegment) api.environment.invokeExact(args[5], args[6], args[7], args[9]);
            require(!handle.equals(MemorySegment.NULL), "native environment construction failed");
            // One Java scope per actual native owner, including shared-owner aliases.
            MemorySegment[] original = new MemorySegment[3];
            for (int i = 0; i < 3; ++i) {
                int view = i;
                if (i == 1 && args[7] != 0) continue;
                MemorySegment pointer = (MemorySegment) api.policy.invokeExact(handle, (long) i);
                original[i] = pointer.reinterpret(8, owners[i], ignored -> {
                    try { require(api.command(handle, 3, view) == 0, "owner close failed"); }
                    catch (Throwable e) { throw new RuntimeException(e); }
                }).asReadOnly();
            }
            policies[0] = original[0]; policies[1] = args[7] != 0 ? original[0] : original[1]; policies[2] = original[2];
            if (args[8] != 0) {
                owners[0].close();
                try { policies[0].get(JAVA_LONG, 0); throw new AssertionError("expired cached borrow accepted"); }
                catch (IllegalStateException expected) {}
                require(api.command(handle, 2, 0) == 0, "native owner survived Java close");
            }
        }
        long policy(int view, boolean accessor) throws Throwable {
            if (accessor) { long value = api.command(handle, 2, view); if (value == 0) throw new Expired(); return value; }
            try { return policies[view].get(JAVA_LONG, 0); }
            catch (IllegalStateException e) { throw new Expired(); }
        }
        @Override public void close() {
            try {
                for (Arena arena : owners) if (arena.scope().isAlive()) arena.close();
                MemorySegment counts = scratch.allocate(16, 8); api.finish.invokeExact(handle, counts);
                entries = counts.get(JAVA_LONG, 0); frees = counts.get(JAVA_LONG, 8);
                require(entries == frees, "region lifetime mismatch");
            } catch (Throwable e) { throw new RuntimeException(e); }
            finally { scratch.close(); }
        }
    }
    static final class Expired extends RuntimeException {
        Expired() { super(null, null, false, false); }
    }
    // Host execution scratch, not regional program records or persistent values.
    static final class Frame {
        final long[] values = new long[32];
        final int procedure, receiver;
        final long savedView;
        final ArrayDeque<Integer> tasks = new ArrayDeque<>(), defers = new ArrayDeque<>();
        int action, stage;
        long pending, savedReturn;
        boolean thrown, hasReturn, cleanup;
        Frame(int procedure, int receiver, long savedView) { this.procedure = procedure; this.receiver = receiver; this.savedView = savedView; }
    }
    static final class Machine {
        final Graph graph;
        final Environment environment;
        final boolean accessor;
        final int mapping;
        final ArrayDeque<Frame> frames = new ArrayDeque<>();
        final ArrayList<Event> events = new ArrayList<>(8192);
        long binding, reads, steps, policyReads;
        int peakFrames;
        Machine(Graph graph, Environment environment, int mapping, boolean accessor, long initial) {
            this.graph = graph; this.environment = environment; this.mapping = mapping; this.accessor = accessor; binding = initial;
        }
        long field(boolean procedure, int id, int field) throws Throwable {
            ++reads;
            if (!accessor) return graph.direct(mapping, procedure, id, field);
            ++graph.api.crossings;
            return (long) graph.api.field.invokeExact(graph.handle, (long) mapping, procedure ? 1L : 0L, (long) id, (long) field);
        }
        int node(int id, int field) throws Throwable { return Math.toIntExact(field(false, id, field)); }
        int procedure(int id, int field) throws Throwable { return Math.toIntExact(field(true, id, field)); }
        void tasks(Frame frame, int head) { if (head != 0) frame.tasks.push(head); }
        void push(int method, long[] arguments, int receiver) throws Throwable {
            require(arguments.length == procedure(method, 1), "arity mismatch");
            Frame frame = new Frame(method, receiver, binding);
            for (int i = 0; i < arguments.length; ++i) frame.values[procedure(method, 3 + i)] = arguments[i];
            tasks(frame, procedure(method, 0)); frames.push(frame); peakFrames = Math.max(peakFrames, frames.size());
        }
        void stage(Frame frame, int kind) throws Throwable {
            frame.stage = kind; frame.tasks.clear();
            if (kind == 2) frame.values[procedure(frame.procedure, 2)] = frame.pending;
            tasks(frame, node(frame.action, kind + 2));
        }
        void nextAction(Frame frame) throws Throwable {
            frame.cleanup = true; frame.tasks.clear();
            if (frame.defers.isEmpty()) { frame.stage = 4; frame.action = 0; return; }
            frame.action = frame.defers.pop();
            stage(frame, frame.thrown ? (field(false, frame.action, 1) != 0 ? 2 : 3) : 1);
        }
        void abrupt(Frame frame, boolean thrown, long value) throws Throwable {
            frame.thrown = thrown; frame.pending = value;
            if (!thrown) { frame.hasReturn = true; frame.savedReturn = value; }
            frame.tasks.clear();
            if (!frame.cleanup) nextAction(frame);
            else if (frame.stage == 1 && thrown && field(false, frame.action, 1) != 0) stage(frame, 2);
            else if (frame.stage != 3) stage(frame, 3);
            else nextAction(frame);
        }
        long eval(int id, Frame frame) throws Throwable {
            return switch (node(id, 0)) {
                case 1 -> field(false, id, 1);
                case 2 -> frame.values[node(id, 2)];
                case 3 -> eval(node(id, 3), frame) + eval(node(id, 4), frame);
                case 4 -> eval(node(id, 3), frame) - eval(node(id, 4), frame);
                case 5 -> eval(node(id, 3), frame) % eval(node(id, 4), frame);
                case 6 -> eval(node(id, 3), frame) == eval(node(id, 4), frame) ? 1 : 0;
                case 7 -> eval(node(id, 3), frame) != 0 ? eval(node(id, 4), frame) : eval(node(id, 5), frame);
                case 8 -> binding;
                case 9 -> { ++policyReads; yield environment.policy((int) field(false, id, 1), accessor); }
                default -> throw new AssertionError("unknown expression");
            };
        }
        long[] arguments(int head, Frame frame) throws Throwable {
            ArrayList<Long> result = new ArrayList<>();
            for (int id = head; id != 0; id = node(id, 6)) result.add(eval(id, frame));
            long[] values = new long[result.size()];
            for (int i = 0; i < values.length; ++i) values[i] = result.get(i);
            return values;
        }
        long[] run(long[] inputs) throws Throwable {
            push((int) graph.layout[14], inputs, 0);
            while (!frames.isEmpty()) {
                ++steps; Frame frame = frames.peek();
                if (frame.tasks.isEmpty()) {
                    if (!frame.cleanup) { nextAction(frame); continue; }
                    if (frame.stage == 1) { stage(frame, 3); continue; }
                    if (frame.stage == 2) { frame.thrown = false; frame.pending = frame.savedReturn; stage(frame, 3); continue; }
                    if (frame.stage == 3) { nextAction(frame); continue; }
                    require(frame.stage == 4, "invalid completion stage");
                    boolean thrown = frame.thrown || !frame.hasReturn;
                    long value = frame.thrown || frame.hasReturn ? frame.pending : 903;
                    binding = frame.savedView; frames.pop();
                    if (frames.isEmpty()) return new long[]{thrown ? 1 : 0, value, binding, environment.api.command(environment.handle, 4, 0), environment.api.command(environment.handle, 5, 0)};
                    Frame parent = frames.peek();
                    if (thrown) abrupt(parent, true, value); else parent.values[frame.receiver] = value;
                    continue;
                }
                int id = frame.tasks.pop(); tasks(frame, node(id, 6));
                try {
                    switch (node(id, 0)) {
                        case 10, 11 -> frame.values[node(id, 2)] = eval(node(id, 3), frame);
                        case 12 -> tasks(frame, eval(node(id, 3), frame) != 0 ? node(id, 4) : node(id, 5));
                        case 13 -> abrupt(frame, false, eval(node(id, 3), frame));
                        case 14 -> abrupt(frame, true, eval(node(id, 3), frame));
                        case 15 -> events.add(new Event(eval(node(id, 3), frame), eval(node(id, 4), frame), eval(node(id, 5), frame)));
                        case 16 -> binding = eval(node(id, 3), frame);
                        case 17 -> {
                            long token = environment.api.command(environment.handle, 0, 0);
                            if (token != 0) frame.values[node(id, 2)] = token; else abrupt(frame, true, 904);
                        }
                        case 18 -> require(environment.api.command(environment.handle, 1, eval(node(id, 3), frame)) == 0, "invalid owner release");
                        case 19 -> push((int) field(false, id, 1), arguments(node(id, 3), frame), node(id, 2));
                        case 20 -> {
                            long view = eval(node(id, 3), frame); require(view >= 0 && view < 3, "invalid interpretation");
                            push((int) graph.layout[15 + (int) view], arguments(node(id, 4), frame), node(id, 2));
                        }
                        case 21 -> frame.defers.push(id);
                        default -> throw new AssertionError("unknown statement");
                    }
                } catch (Expired e) { abrupt(frame, true, 905); }
            }
            throw new AssertionError("missing root result");
        }
    }
    static List<Case> cases(Path path) throws Exception {
        List<String> lines = Files.readAllLines(path); ArrayList<Case> cases = new ArrayList<>();
        for (int index = 0; index < lines.size();) {
            String[] header = lines.get(index++).split(" ");
            require(header[0].equals("CASE"), "case format");
            long[] arguments = new long[10], result = new long[5];
            for (int i = 0; i < 10; ++i) arguments[i] = Long.parseLong(header[i + 2]);
            for (int i = 0; i < 5; ++i) result[i] = Long.parseLong(header[i + 12]);
            int count = Integer.parseInt(header[17]); ArrayList<Event> events = new ArrayList<>(count);
            for (int i = 0; i < count; ++i) {
                String[] event = lines.get(index++).split(" ");
                events.add(new Event(Long.parseLong(event[0]), Long.parseLong(event[1]), Long.parseLong(event[2])));
            }
            cases.add(new Case(header[1], arguments, result, events));
        }
        return cases;
    }
    static final com.sun.management.ThreadMXBean ALLOCATIONS = (com.sun.management.ThreadMXBean) ManagementFactory.getThreadMXBean();
    static long allocated() { return ALLOCATIONS.getThreadAllocatedBytes(Thread.currentThread().threadId()); }
    static void execute(Graph graph, Case sample, String mode, int repetition, boolean report) throws Throwable {
        API api = graph.api; int mapping = repetition % 2;
        long nanoseconds, bytes, crossings, reads = 0, steps = 0, policies = 0; int peak = 0;
        List<Event> events; long[] result; Environment environment = new Environment(api, sample.arguments());
        try (environment) {
            long[] inputs = Arrays.copyOf(sample.arguments(), 5);
            if (mode.equals("native")) {
                MemorySegment in = environment.scratch.allocateArray(JAVA_LONG, inputs);
                MemorySegment out = environment.scratch.allocate(40, 8);
                long before = allocated(), start = System.nanoTime();
                api.nativeRun.invokeExact(environment.handle, in, out);
                nanoseconds = System.nanoTime() - start; bytes = allocated() - before; crossings = 1;
                result = out.toArray(JAVA_LONG);
                MemorySegment trace = environment.scratch.allocate(8192 * 24, 8);
                long count = (long) api.events.invokeExact(environment.handle, trace, 8192L * 3);
                events = new ArrayList<>((int) count);
                for (int i = 0; i < count; ++i) events.add(new Event(trace.get(JAVA_LONG, i * 24L), trace.get(JAVA_LONG, i * 24L + 8), trace.get(JAVA_LONG, i * 24L + 16)));
                policies = api.command(environment.handle, 6, 0);
            } else {
                Machine machine = new Machine(graph, environment, mapping, mode.equals("accessor"), sample.arguments()[5]);
                api.crossings = 0; long before = allocated(), start = System.nanoTime();
                result = machine.run(inputs);
                nanoseconds = System.nanoTime() - start; bytes = allocated() - before; crossings = api.crossings;
                events = machine.events; reads = machine.reads; steps = machine.steps; peak = machine.peakFrames; policies = machine.policyReads;
            }
            require(Arrays.equals(result, sample.result()), sample.name() + "/" + mode + " result " + Arrays.toString(result) + " expected " + Arrays.toString(sample.result()));
            require(events.equals(sample.events()), sample.name() + "/" + mode + " complete trace differs");
        }
        long expectedRegions = 3 + sample.result()[3] + (sample.arguments()[6] >= 1 && sample.arguments()[6] <= sample.arguments()[0] ? 1 : 0);
        require(environment.entries == expectedRegions, "unexpected root count");
        if (report) System.out.printf(Locale.ROOT, "ROW %s %s %d %d %d %d %d %d %d %d %d %d %d %d%n", sample.name(), mode, repetition, mapping,
                nanoseconds, bytes, crossings, reads, steps, peak, policies, events.size(), environment.entries, environment.frees);
    }
    public static void main(String[] arguments) throws Throwable {
        require(arguments.length == 4, "library, fixture path, cases path, repetitions");
        List<Case> cases = cases(Path.of(arguments[2])); int repetitions = Integer.parseInt(arguments[3]);
        require(ALLOCATIONS.isThreadAllocatedMemorySupported(), "thread allocation measurements unavailable");
        if (!ALLOCATIONS.isThreadAllocatedMemoryEnabled()) ALLOCATIONS.setThreadAllocatedMemoryEnabled(true);
        try (Arena library = Arena.ofConfined()) {
            API api = new API(Path.of(arguments[0]), library);
            try (Graph graph = new Graph(api, Path.of(arguments[1]))) {
                Case warmup = cases.stream().filter(c -> c.name().equals("depth-20-source-1")).findFirst().orElse(cases.get(0));
                for (int round = 0; round < 10; ++round) for (String mode : List.of("direct", "accessor", "native")) execute(graph, warmup, mode, round, false);
                Random order = new Random(71);
                for (int repetition = 0; repetition < repetitions; ++repetition) for (Case sample : cases) {
                    List<String> modes = new ArrayList<>(List.of("direct", "accessor", "native")); Collections.shuffle(modes, order);
                    for (String mode : modes) execute(graph, sample, mode, repetition, true);
                }
                System.out.printf("GRAPH %d %d %d%n", graph.original.length, graph.nodes.length, graph.procedures.length);
            }
        }
        System.out.println("PASS managed regional control, complete oracle traces, owner scopes, readonly/bounds/closed guards, and exact region frees");
    }
}
