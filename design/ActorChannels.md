# Actors and typed chan: network and cluster research

Design and research revision: 11 · 2026-11-28

## Design authority

At r11 the [transfer dispatch decision](../CHANGELOG.md#h-swarm-transfer-specialization-20261128) confirms static rejection when a provider has no matching transfer handler. The intended declaration is a method specialization dispatched on `swarm type XXX`. P-789 continues actual-swarm ownership and joint placement for the launch. The table below illustrates method selection.

At r10 two [launch examples](#acceptance-scenarios) apply the existing rules for consuming calls, parameter defaults and fresh TLS to the r9 named entry. P-788/P-789 continue. [Scope and continuity](../CHANGELOG.md#k-swarm-launch-preparation-20261128)

At r9 the [worker identity decision](../CHANGELOG.md#h-provider-private-workers-20261128) keeps physical workers private to the swarm provider. D-32-RESOURCE-POOLS/P-789 continues the declared handoff requirements for a complete launch. The source-level example and the cluster workload below use a swarm, entry arguments and application message endpoints.

At r8 the [provider handoff direction](../CHANGELOG.md#h-swarm-provider-handoff-20261128) assigns inter-worker ownership transfer to the swarm provider interface. The provider declares which transfers it supports. P-789 continues same-swarm authority and the independent chan rule. The declaration model below is a proposal for static checking and dynamic worker selection.

At r7 the [swarm and chan decision](../CHANGELOG.md#h-swarm-implementations-and-chan-payloads-20261128) admits thread pools, local process pools and dynamically recruited cross-machine process pools. D-32-RESOURCE-POOLS/P-789 limits swarm-region handoff to its swarm. D-32-REGION-TRANSPORT/P-711 admits only ordinary regions as chan payloads. This supersedes the r5–r6 same-pool chan example.

At r6 the [region and packet decision](../CHANGELOG.md#h-region-locality-and-packet-chans-20261128) distinguishes ordinary region handoff from packet delivery. D-32-RESOURCE-POOLS/P-789 covers local and pool-bound ordinary regions. D-32-REGION-TRANSPORT/P-711 continues whole-packet validation and adds swarm-independent typed chan delivery. The source spellings below are proposals.

At r5 the [resource pool direction](../CHANGELOG.md#h-go-resource-pools-20261128) requires an actual target pool for every go process and makes the pool a first-class ownership boundary for both r4 launch forms. The same-host shared-memory example examines feasibility. Pool construction, placement policy and endpoint syntax remain open for `e-wrks/cod`.

At r4 the [named launch decision](../CHANGELOG.md#h-go-owning-parameters-20261128) extends the r3 ownership example to the named entry procedure and its consuming parameters. Both forms use the same dedicated thread and fresh-TLS boundary under D-10-ACTOR-CHAN/P-788.

At r3 the [heavy-task and go process decision](../CHANGELOG.md#h-heavy-tasks-and-go-process-20261128) defines local Actor launch and fresh initial TLS in the production language. The r2 endpoint and whole-region transport scopes continue. P-702/P-788 distinguish launching local work from delivering work to another machine.

[Regional transport](DesignAuthorityRegistry.md#d-32-region-transport) and
[chan endpoints](DesignAuthorityRegistry.md#d-32-chan-ends) record the two confirmed planning scopes
at r2. P-711/P-712 link them to the packet and pipeline contracts. The acceptance cases below cover
whole-region visibility under loss, capacity pressure and independent use of each endpoint.
[Decision and continuity](../CHANGELOG.md#h-actor-region-transport-and-chan-ends-20261124)

[D-32](DesignAuthorityRegistry.md#d-32) records this user-directed research.
[D-10-ACTOR-CHAN](DesignAuthorityRegistry.md#d-10-actor-chan) owns the confirmed
[Actor and chan boundary](SymbolicDynamicScope.md#actor-concurrency-direction): language terminology,
one dedicated OS thread per Actor, isolated dynamic context, static message typing and fire-and-forget
sending except for reported systemic communication faults. The
[human decision](../CHANGELOG.md#h-actor-chan-boundary-20261123) records the scope and its exclusions.

The remaining interfaces, ownership transitions and application protocols below are research
candidates, **pending-human**. P-703 records their dependencies; P-704 retains a candidate value path
through C-01/C-02/C-52 and the charter's separation of program, execution and interpretation
(§§3.2–3.3/6.1–6.3). P-131–P-133 retain their existing qualification. The value is an inspectable
typed workload that can be placed locally or on a cluster without hiding storage, authority or
business completion behind a transport operation.

Preserve actual owners, same-region stored edges, callable placement, scoped dynamic bindings and
cleanup. Consult the scenario matrix for acceptance and the open decisions for the remaining wire,
queue, ownership and source-language choices. The
[continuity record](../CHANGELOG.md#k-actor-chan-research-20261123) identifies the source baseline and
the older execution and send-result candidates replaced by the new decisions.

## Confirmed basis

A **go process** starts an Actor's execution in a target resource pool on a dedicated OS thread.
The target is required for both a named entry procedure with explicit arguments and a
double-braced body with an owner list. Each launch begins with
fresh initial TLS. The launcher keeps its own active bindings. Several such executions can run in
one OS process. **Chan** is the typed messaging facility. For example, a cluster coordinator can
send work to an existing Actor on another machine, where local code can launch a go process to
handle a partition. While the go process runs, it occupies a dedicated OS thread on its worker. A reused worker starts the new invocation with fresh initial TLS.
[Launch forms and effects boundary](SymbolicDynamicScope.md#go-process-launch)

A chan has a **statically checked message type**. Sender and receiver code must satisfy that type's
contract. A received wire representation still needs validation before it can become a typed value;
an address, method name or structurally similar payload does not establish its CoD type identity.
A chan must allow each endpoint to be modeled independently. The endpoint declarations and carriers
will be specified separately.

**Ordinary sending is fire-and-forget.** The facility does not guarantee reception and supplies no
delivery receipt or acknowledgement. The sender must receive a system-fault report when the system
has grounds to judge that subsequent sends are likely to fail. An individual lost message, rejected
job or failed computation is not automatically such a systemic fault. The detection criteria,
reporting point and error representation remain to be specified.

The selected inter-Actor network path uses an AKCP stream over UDP for semi-reliable region
transfer. Applications define any further ordering, acknowledgement, retry, duplicate handling or
reliable stream their work requires. A reply travels as another application message with its own
delivery risk.

| Observation | Meaning at the confirmed boundary |
| --- | --- |
| A send completes without a system-fault report | The sender obtains no delivery or processing evidence and need not follow that individual transmission. |
| A systemic communication fault is reported | The communication facility has a fault expected to compromise subsequent sending; its precise scope and evidence need a contract. |
| A deadline expires while an application awaits a reply | The application has not observed the required reply; execution and external effects may already have happened. |
| A worker sends a result, refusal or cancellation response | This is an application protocol message, not a chan-generated receipt. |
| A connection closes | A transport observation. Its relation to an Actor, chan, in-flight work and system-fault reporting must be defined explicitly. |

## Resource pools and ownership handoff

A **swarm** is a first-class resource pool. Every `go` launch names a target swarm. A swarm can supply a pool of OS threads in one process, a pool of OS processes on one host, or a pool of worker processes dynamically recruited across machines. Passing a swarm to a library preserves its identity. Equally configured swarms have distinct identities. The implementation determines where work runs and which physical transports it supports.

CoD source names the swarm and the entry to run. A **worker** is a provider-private means of executing that entry. CoD exposes no worker object, handle or selectable worker identity. A provider may identify its own threads or processes for scheduling, recovery and tracing. A worker identifier in a trace carries no CoD authority to direct a later launch or transfer an owner.

A producer can hand a **swarm region** to a go process through an owning entry parameter or the block’s owner list. The producer, resource and launched go process must satisfy the same actual swarm requirement. The provider must place the entry where it can use and release that resource. For example, shared memory can support a handoff to a local subprocess. Recruiting a remote worker into the same swarm establishes membership, but the local mapping cannot give that worker access to the bytes.

A typed `chan` carries **only ordinary regions**. A packet is an ordinary region used as a message and has no swarm association. The same rule applies when sender and receiver belong to the very same swarm, even when both are threads in one OS process. A swarm region is ineligible as a chan payload, including inside a wrapper.

### Declaring a swarm provider’s handoff operations

The **swarm provider** exposes the operations that transfer ownership to the launched go process and declares which transfers it supports. Both owning launch forms use this interface. The resource’s own contract still determines who can use and release it. Ordinary synchronous owner passing within one go process continues to use the ordinary call rules. Typed chan sends have their own message contract.

**Transfer support is checked statically.** The provider publishes the transfers it implements. If an owning argument has no applicable handler in that interface, the compiler rejects the launch at the argument. Omitting a handler suffices to leave that case unsupported. This check applies to both owning parameters of a named entry and the block’s owner list.

The proposed declaration form is a transfer method specialized on the swarm type and the resource it accepts. For example, `swarm type LocalProcessPool` would identify the provider type for dispatch. Its signature identifies the accepted resource and can require particular backing or placement. For known types, ordinary method specificity selects among applicable specializations. An ambiguous best match is a static error. A same-named free method outside the published provider interface supplies no transfer permission. The examples show dispatch outcomes. The exact method head and spelling of its conditions remain open. [Receiver method families](DomainLanguageDesign.md#how-receiver-dispatch-works)

| Swarm type | Owning argument and applicable placement | Result at the launch |
| --- | --- | --- |
| `LocalProcessPool` | `part`, a swarm region with relocatable shared-memory backing, between processes on one host | Select the provider’s shared-region transfer specialization. |
| `LocalProcessPool` | `sink`, an `IndexFile` handle with a transferable descriptor, between processes on one host | Select the provider’s file-descriptor transfer specialization. |
| `ClusterPool` | The same `sink` for a cross-host launch, with no applicable specialization in this provider | Reject at `sink` during compilation. |

Two separately created swarms of type `LocalProcessPool` can use the same transfer method. A region belonging to the first still cannot be handed to an entry launched in the second. Dispatch identifies an operation. The owning launch also requires the same actual swarm.

Swarm identity, the resource’s actual owner, and the provider declaration answer separate questions. Two regions with the same roots can have different backing. A matching structural schema proves no shared-memory capability. For a nominal resource, an equally shaped type from another declaration cannot satisfy a specialization naming the original type. On success the selected operation makes the resource usable at the recipient with one release responsibility. Its declaration alone cannot guarantee successful execution. [Nominal resource identity](GraphGrainedMemoryManagement.md#opaque-nominal-region-types) · [Owning inputs at launch](SymbolicDynamicScope.md#go-process-launch)

Consider `go on(s) build_index(part, sink, id)`, using the candidate launch spelling. The entry consumes a swarm region and a handle. The compiler checks the provider’s declared support for both owning arguments under one compatible placement. Two individually supported transfers that require incompatible placements cannot justify this launch. On admission the provider must arrange for the same child to receive both owners and maintain the placement conditions through handoff. Recruitment or replacement can change how the provider meets that obligation without changing the call or its types. Temporary lack of capacity for a supported launch is a separate issue. The wait or failure policy and the ownership outcome after an admitted handoff fails remain open.

### Process-local regions, pool-bound regions and packets

| Use | Association | Rule |
| --- | --- | --- |
| Ordinary region for local work | Omit the swarm at construction. | A procedure in the same go process can borrow, consume or return it. Running inside a swarm gives a local region no implicit association. |
| Ordinary region sent as a packet | Independent of swarms. | If it satisfies the chan’s message contract, the endpoint can send it within or across swarms. Reception publishes the whole validated packet. |
| Swarm region | Belongs to one actual swarm. | An owning launch can hand it off within that swarm when its transport permits. Every chan rejects it. |

For example, a coordinator in one pool sends a packet of keys to a worker in another. The worker interprets those keys using its own index. The packet needs no pool identity at construction, in transit or on receipt. The coordinator keeps its swarm index region. Sending that index itself would fail even to another worker in the coordinator’s swarm. The [packet contract](DomainLanguageDesign.md#dl-04) governs the delivered representation and its compatibility. The [chan contract](#confirmed-basis) governs send feedback.

**Candidate source spelling.** `swarm s` would declare a first-class resource pool with its own identity. On region construction, `within s` would create a swarm region associated with that identity. Compare these constructions in a producer already participating in `s`:

```cod
own region scratch <- make region Partition(bytes -> capacity) { count <- 0 }
own region batch <- make region within s Partition(bytes -> capacity) { count <- 0 }
go on(s) build_index(batch)
```

Assume `Partition` has a single `count` root and the entry procedure takes its owner. Passing `scratch` at the same launch would fail, even if both threads use the same address space. Passing `batch` to a distinct pool would also fail. In the candidate grammar, `within` qualifies the creation of the whole region. A nested `make Node in r { ... }` still places an object in its actual owner `r`. Both `swarm` and `within` can be contextual. The declaration and construction spellings, who may originate a region for a newly declared pool, and the source form for creating a packet remain open.

### A shared-memory region between local processes

A swarm region with the structural `Partition` schema can use OS shared memory for an owning launch to another process on the same host. Consider one partition whose nodes refer to each other through region-relative locators. The parent finishes its writes. A local subprocess maps the same backing at its own base address and resolves the same root, shared nodes and cycles. The handoff must publish the completed writes before the receiving entry can use the region. Linux shared-memory objects and descriptor passing provide the native pieces for such a transport. The source root schema can stay the same. [Regional graph representation](GraphGrainedMemoryManagement.md#one-region-per-pointer-graph) · [POSIX shared memory on Linux](https://man7.org/linux/man-pages/man7/shm_overview.7.html) · [Unix-domain descriptor passing](https://man7.org/linux/man-pages/man7/unix.7.html)

Mapping the bytes and transferring the owner are separate events. After successful handoff the sender’s consumed binding and stale sender access paths cannot use the region, even if its native mapping still exists. The receiving side has the release obligation. A process-local native pointer or handle requires its own valid transport before the receiving program can use it. The mapping alone does not synchronize concurrent mutation or establish a lifetime for an external resource.

**Design probes**

| Case | Outcome or decision |
| --- | --- |
| Niladic entry or owner-free block with no target pool | Reject the launch. Having no ownership to check does not supply a target. |
| Owning launch within one swarm, different OS processes on one host, relocatable shared backing | Permit the swarm-region handoff. Verify a different mapping base, preserved graph edges, visible completed writes and one release responsibility. |
| Two threads in one swarm send the swarm’s region through a typed chan | Reject the send even when the root matches the message schema and both threads can address the bytes. |
| Owning launch across two distinct swarms in the same address space | Reject transfer of the swarm region. |
| A recruited worker on another machine receives an ordinary packet via chan | Apply ordinary packet type and whole-region validation. Swarm membership supplies no mapping for a host-local resource. |
| Same pool, subprocess receiver, opaque handle usable only in the sender process | Pool identity meets one condition. The handle still lacks a usable subprocess transport. |
| Work is accepted into a queue and the receiving worker dies | The responsible queue or transport must account for the resource under the eventual failure contract. An ordinary chan send still gives no delivery receipt. |
| Two pools need the same business data | Send a self-contained packet over a typed chan. The receiver may use the packet or construct its own ordinary region from the data. The sender’s swarm region stays in its swarm. |

How a distributed swarm authenticates recruited workers, places work and handles worker loss remains open. The same resource and chan rules apply regardless of worker placement. The seed and booter retain their bootstrap scope.

## Regional transport over AKCP

The network transfer unit visible to the receiving Actor is one whole packet region. The packet is an ordinary region. Typed chan delivery works within a swarm or across swarms. A sender may submit a
region of any finite length that the peers can represent and have resources to handle. The carrier
can make progress across many UDP datagrams, AKCP records and repair windows. Bytes can arrive,
be repaired or become permanently unavailable while a region is in progress. The runtime may
publish the typed region only after it has its complete extent and has validated the representation
against the receiving endpoint. If the transfer fails, the Actor obtains no part of the region and
its entry cannot run. Any private assembly storage still has an owner responsible for release.
[D-07 packet length and compatibility](DomainLanguageDesign.md#dl-04)

**What AKCP supplies.** VNP identifies bytes by direction, stream epoch and offset. Its receiver can
repair a gap for a bounded time using retransmission and forward error correction. If the gap
persists, it can skip affected frame records and resume at a verified record boundary. Late bytes
cannot resurrect an abandoned record. The receiver distinguishes known missing data from an
unobserved tail. The carrier offers complete frames to its delivery callback. It handles byte
acknowledgement and capacity credit below that callback. VNP's session callback receives only a
complete frame. A region adapter using that API needs enough metadata in surviving records to
recognize missing pieces and to identify the next region.
[VNP stream and gap rules](https://github.com/daowei2026/vnp/blob/315634cf92cfb322f91788e295cffefb0c65825b/docs/protocol/akcp-v1.md#L6-L31) ·
[Record boundaries and recovery](https://github.com/daowei2026/vnp/blob/315634cf92cfb322f91788e295cffefb0c65825b/docs/protocol/akcp-v1.md#L111-L145) ·
[VNP delivery callback](https://github.com/daowei2026/vnp/blob/315634cf92cfb322f91788e295cffefb0c65825b/internal/session/carrier.go#L696-L722)

**What the CoD region protocol must supply.** VNP v1 admits Ethernet records of 14 to 65,535 bytes.
Its sender also checks that one complete record fits in its TX ring. The region protocol therefore
needs framing above those records or a distinct region framing profile on the AKCP stream. It must
recover the next region boundary even when the first record of the damaged region was lost. It must
also recognize a missing last piece before publication. The length in the CoD packet header and
the manually assigned root compatibility identifier supply part of this agreement. The protocol must decide
how to identify the transfer, establish complete byte coverage, validate the root and distinguish
a fresh transfer from stale bytes. The current wire layout and encoding of those facts remain open.
[VNP sender admission](https://github.com/daowei2026/vnp/blob/315634cf92cfb322f91788e295cffefb0c65825b/internal/transport/ring.go#L96-L115) ·
[CoD packet contract](DomainLanguageDesign.md#dl-04)

For example, suppose region A spans three records and region B follows. If repair recovers the
middle record in time, A can pass validation and be published whole. If that record stays missing,
A must be discarded in full. Once the receiver can establish B's boundary and validate B, it can
offer B independently. A late piece of A must never complete B or reopen the discarded assembly.
For a missing final record with no later traffic, the receiver may have insufficient evidence to
identify a remote loss. Its retirement policy must eventually reclaim any retained partial assembly.
The sending Actor obtains no delivery receipt in any of these cases. A result or durable business
commit is observable only through an application protocol.

### Queues and backpressure

The transport must support queuing and backpressure. When a receiving Actor stops consuming
complete regions, retained output consumes capacity. The transport must be able to slow further
admission or transmission and resume when capacity becomes available. An application may still
lose a region under the selected retention policy. A slow consumer and a stream hole need distinct
accounting because capacity becoming available can resolve the former without repairing any bytes.

The capacity budget must account for queued sends, transport repair windows, private region
assembly and complete regions awaiting their receiving Actor. An implementation
must be able to stage a region larger than a single transport window. Holding every byte of credit
until the whole region is published would stall such a transfer. One candidate is to move validated
pieces into separately budgeted, private region storage and advance transport credit as its slots
are released. The assembly and ready queues then impose their own bounds. VNP uses a directional
credit limit for available receive capacity. A credit update permits further sending and says
nothing about which regions a receiving Actor observed.
[VNP capacity and backpressure](https://github.com/daowei2026/vnp/blob/315634cf92cfb322f91788e295cffefb0c65825b/docs/protocol/relay-v2.md#L56-L72)

The queue accounting unit, admission timing, behavior at capacity, expiry and recovery reporting
need a policy. Temporary pressure does not by itself establish the continuing communication fault
that requires reporting to a sending Actor. The send-side ownership and fault contract must define
who holds a region while admission waits, after admission, and if a systemic fault is reported.
[Ownership on a systemic fault](GraphGrainedMemoryManagement.md#ownership-when-a-systemic-fault-is-reported)

## Independent chan endpoints

A component can be modeled with only the statically typed sending end or only the receiving end.
From the local pipeline's perspective the sending end plays the **consumer** role for its message
type. The receiving end plays the **producer** role when it offers locally admitted messages.
A **transformer** can adapt a local source before sending or adapt the received values before a
local consumer sees them. For example, a worker can hold only the sending end of a result channel,
while the collector uses only the receiving end. Neither operation needs to be modeled as owning
the other end.

CoD will express these roles in a formal producer/transformer/consumer type language once it supports
**higher-kinded type synonyms**. For the endpoint workload the type language must let two components
describe their own roles and let a caller check the message family at composition. The declarations,
kind rules and concrete endpoint signatures are future work. In a local synchronous traversal,
feedback belongs to that traversal. A remote reply is a separate application message. An async
queue may retain admissible payloads and endpoint capabilities under their eventual ownership rules.
It cannot retain a borrowed `here` activation.
[Named signatures and pipelines](../docs/DesignTopics.md#named-signatures-and-pipelines) ·
[Producer and transformer contracts](ProducersAndTransformers.md#design-authority)

## Reference baselines and method

The project user selected nedh and sedh for their application workloads and VNP for its transport:

| Repository | Inspected default-branch revision | Role in this study |
| --- | --- | --- |
| [VNP](https://github.com/daowei2026/vnp/tree/315634cf92cfb322f91788e295cffefb0c65825b) | `315634cf92cfb322f91788e295cffefb0c65825b` | AKCP over UDP, record recovery, queuing and receive capacity. |
| [nedh](https://github.com/complyue/nedh/tree/04f5e0f62aabe656c21c3181edd9ba72c3f0c5b7) | `04f5e0f62aabe656c21c3181edd9ba72c3f0c5b7` | Network sessions, transport adapters, routed channels and request/reply workloads. |
| [sedh](https://github.com/complyue/sedh/tree/2567b52ed58985d0edfa6312a3b883c5cf8b79ab) | `2567b52ed58985d0edfa6312a3b883c5cf8b79ab` | Work-source discovery, resource recruitment, worker management, batch execution and result settlement. |

The findings below come from source inspection, including examples. They are not deployment or
performance measurements. A reference implementation is evidence for a workload and its existing
mechanisms; it does not approve a CoD API. Explicitly identified inconsistencies limit what can be
claimed from an example.

## What nedh contributes

### A connection, a route and a local execution context have different identities

A nedh `Peer` combines a connection's send/receive operations, end-of-life state, a channel map and
the environment in which incoming commands land. In the Haskell implementation the map is keyed by
an `EdhValue`. Text payloads and route expressions are evaluated; a directed result is written to the
selected BChan. Python similarly runs source in the supplied globals/locals. An undirected command
returns its result from the landing operation; it is not evidence of another dedicated inbox.
[Haskell Peer](https://github.com/complyue/nedh/blob/04f5e0f62aabe656c21c3181edd9ba72c3f0c5b7/host.hs/src/Language/Edh/Net/Peer.hs#L18-L110) ·
[Python landing](https://github.com/complyue/nedh/blob/04f5e0f62aabe656c21c3181edd9ba72c3f0c5b7/host.py/nedh/peer.py#L94-L186)

For CoD, this motivates separate contracts for Actor identity, chan endpoint identity, network route
and the receiver's admitted program interpretation. A TCP connection must not silently become the
Actor's identity, nor may a route string establish authority to invoke an arbitrary procedure.
Candidate work messages should describe an admitted operation and its inputs. The receiver supplies
the handlers and live resources required for that interpretation. A code-carrying protocol would
need its own checking and admission boundary.

nedh's design note recommends locally managed objects and communication through channels, avoiding a
distributed object-reference lifetime protocol. That is useful business guidance for CoD's
region-local graphs and explicit cross-region identities. The note's wish to tolerate late messages
to retired channels is not uniformly implemented: the Haskell/Python lookup paths reject a missing
channel. These are different evidence levels.
[Distributed-concurrency note](https://github.com/complyue/nedh/blob/04f5e0f62aabe656c21c3181edd9ba72c3f0c5b7/DistConc.md)

### Transport framing and queue resources must be specified independently

The TCP micro-protocol sends a byte-length/directive header followed by payload bytes. WebSocket
uses a routing frame followed by a payload frame; browser MessageChannel uses MessagePort envelopes.
The UDP discovery path operates on individual datagrams. These adapters share an application purpose
without supplying one interchangeable framing or lifetime contract.
[TCP framing](https://github.com/complyue/nedh/blob/04f5e0f62aabe656c21c3181edd9ba72c3f0c5b7/host.hs/src/Language/Edh/Net/MicroProto.hs#L29-L93) ·
[WebSocket adapter](https://github.com/complyue/nedh/blob/04f5e0f62aabe656c21c3181edd9ba72c3f0c5b7/host.hs/src/Language/Edh/Net/WebSocket.hs#L294-L395) ·
[MessagePort adapter](https://github.com/complyue/nedh/blob/04f5e0f62aabe656c21c3181edd9ba72c3f0c5b7/host.js/nedh/mc.js) ·
[UDP advertiser/sniffer](https://github.com/complyue/nedh/blob/04f5e0f62aabe656c21c3181edd9ba72c3f0c5b7/host.hs/src/Language/Edh/Net/Sniffer.hs#L180-L249)

Queue behavior also differs. Haskell uses single-slot TMVars on the connection path. Python's client
uses a one-item inbound queue and an unbounded outbound queue; its comment explicitly notes that
outgoing packets can accumulate despite remote backpressure. The browser adapter publishes through
EventSink. A common channel name therefore does not establish a common buffering guarantee.
[Haskell connection queues](https://github.com/complyue/nedh/blob/04f5e0f62aabe656c21c3181edd9ba72c3f0c5b7/host.hs/src/Language/Edh/Net/Server.hs#L207-L295) ·
[Python queues](https://github.com/complyue/nedh/blob/04f5e0f62aabe656c21c3181edd9ba72c3f0c5b7/host.py/nedh/client.py#L87-L100) ·
[Browser channel publication](https://github.com/complyue/nedh/blob/04f5e0f62aabe656c21c3181edd9ba72c3f0c5b7/host.js/nedh/ws-client.js#L145-L167)

CoD needs a resource policy for retained bytes, message counts and disposal. That policy must fit the
confirmed feedback boundary: occasional discard does not turn into a per-message negative receipt.
Application credits or windows may regulate offered traffic. They do not remove the runtime's duty
to bound or account for its own retained resources, and they do not select the basic queue policy.

### Reply endpoints and shared collectors reveal ownership questions

In the web REPL, a session has a shared `cmdIn`. Each client creates a `cmdOut` and submits the pair
`(cmdSrc, cmdOut)`; the session replies through that supplied endpoint. This is a concrete reason to
study typed reply capabilities in request messages, rather than assuming one global response path.
[REPL request and reply](https://github.com/complyue/nedh/blob/04f5e0f62aabe656c21c3181edd9ba72c3f0c5b7/edh_modules/net/repl.edh#L145-L226)

The Factotum example lets multiple worker connections share `resultCollector` and uses
`dispose=false` so one connection's end does not close the shared collector. A sender, connection
owner, chan owner and party permitted to close it are consequently distinct roles. In contrast,
Python `Factotum.eval` reads a shared uplink without adding a per-request correlation identifier;
that operation alone does not establish safe pairing of concurrent requests.
[Shared collector](https://github.com/complyue/nedh/blob/04f5e0f62aabe656c21c3181edd9ba72c3f0c5b7/Tour/factotum.edh#L25-L68) ·
[Python eval](https://github.com/complyue/nedh/blob/04f5e0f62aabe656c21c3181edd9ba72c3f0c5b7/host.py/nedh/facto.py#L181-L185)

The CoD question is how a request expresses the permitted response type and a valid return route
without embedding a raw pointer into another Actor's region. Endpoint capabilities, correlation
identity, expiry and close authority require their own design. Reusing Edh's name does not adopt its
dynamic lookup, buffering, rendezvous or broadcast behavior.

## What sedh contributes

### Resource recruitment is separate from task execution

sedh's control centre manages node configuration and operational status. Scheduling is coordinated
by a HeadHunter for each work source and a Forager on each participating node. The Forager maintains
the worker team associated with a work-source connection. Its work-source key is derived from the
UDP source address, not from a durable job identity.
[Control centre](https://github.com/complyue/sedh/blob/2567b52ed58985d0edfa6312a3b883c5cf8b79ab/edh_modules/swarm/cc/server.edh#L28-L71) ·
[Work-source registration](https://github.com/complyue/sedh/blob/2567b52ed58985d0edfa6312a3b883c5cf8b79ab/edh_modules/swarm/forager/mgmt.edh#L229-L281)

The inspected path is:

1. HeadHunter advertises `WorkToDo`, including requested headcount, executable, working directory,
   work module and priority.
2. Forager reserves capacity and connects back with `OfferHeads`.
3. HeadHunter sends the employed count on the data channel; Forager starts the corresponding workers.
4. Each worker establishes a direct session with HeadHunter and announces `StartWorking`.
5. HeadHunter assigns a job's parameter pack. The worker executes `doOneJob`, reports settlement and
   sends a result or error through the application channels.

[Recruitment](https://github.com/complyue/sedh/blob/2567b52ed58985d0edfa6312a3b883c5cf8b79ab/host.py/sedh/headhunter.py#L181-L283) ·
[Team creation](https://github.com/complyue/sedh/blob/2567b52ed58985d0edfa6312a3b883c5cf8b79ab/edh_modules/swarm/forager/mgmt.edh#L55-L164) ·
[Worker protocol](https://github.com/complyue/sedh/blob/2567b52ed58985d0edfa6312a3b883c5cf8b79ab/host.py/sedh/worker/__main__.py#L52-L99)

The topology below describes those application roles; it does not map each role to a CoD primitive.

```mermaid
flowchart TB
    H["HeadHunter"] -->|"WorkToDo advertisement"| F["Forager"]
    F -->|"OfferHeads"| H
    H -->|"Employment count"| F
    F -->|"Start and maintain"| W["Worker team"]
    W -->|"StartWorking and results"| H
    H -->|"Job parameters"| W
```

These are distinct protocol phases. An employment confirmation does not confirm a task's execution;
replenishing a worker team does not retry the lost worker's job. For CoD, both confirmations and
results would be typed application messages sent through chan, subject to the application's own
loss-handling policy.

### Scheduling policy belongs to the application workload

Forager's queue groups work sources by priority, with FIFO behavior within a priority bucket. Each
bucket has a configured capacity and may discard an old entry when full; repeated discovery offers
another chance to engage. HeadHunter's pending jobs and idle workers use FILO lists. The resource
being recruited is headcount, rather than a general memory/GPU/data-locality placement model.
[Work-source queues](https://github.com/complyue/sedh/blob/2567b52ed58985d0edfa6312a3b883c5cf8b79ab/edh_modules/swarm/forager/wsq.edh#L36-L180) ·
[HeadHunter state](https://github.com/complyue/sedh/blob/2567b52ed58985d0edfa6312a3b883c5cf8b79ab/host.py/sedh/headhunter.py#L94-L120)

The useful CoD workload is an independently typed resource-offer protocol coupled to a typed job
protocol. Priority, fairness, batching, placement and capacity reservations are application choices.
The Actor-to-thread mapping is already fixed; cluster scheduling should decide where work and Actors
are placed, without introducing a second language-level thread model.

### Failure, retry, settlement and stopping are separate events

The native worker launcher connects a socket and then fork/execs an OS subprocess, passing the socket
descriptor. Losing the management connection leads Forager to stop its worker team; the native
stop helper sends SIGKILL. This is an OS-process mechanism in sedh, not evidence for safely terminating
one CoD Actor's thread inside a shared OS process.
[Native worker lifecycle](https://github.com/complyue/sedh/blob/2567b52ed58985d0edfa6312a3b883c5cf8b79ab/host.hs/src/Language/Edh/Swarm/Worker.hs#L50-L124) ·
[Team stop](https://github.com/complyue/sedh/blob/2567b52ed58985d0edfa6312a3b883c5cf8b79ab/edh_modules/swarm/forager/mgmt.edh#L166-L185)

A HeadHunter job timeout closes that worker's peer. The code does not obtain a cancellation-completed
acknowledgement. Retry policy is separately selected; the inspected path has no durable job journal
or result-deduplication transaction. If an effect happened before a reply was lost, retry can repeat
it. This is a consequence of the path, not an asserted test result.
[Job timeout and retry](https://github.com/complyue/sedh/blob/2567b52ed58985d0edfa6312a3b883c5cf8b79ab/host.py/sedh/headhunter.py#L300-L409)

CoD needs to distinguish a cancellation request, observing a cancellation response, Actor exit and
completion of resource cleanup. Under fire-and-forget sending, even the cancellation request may
be lost. A stream or job protocol may provide a typed response stating exactly what has stopped;
that does not create a basic send receipt or undo earlier effects.

The examples include independent parameter-grid jobs, reuse of a HeadHunter over several batches,
and sessions retaining worker-local state. Shared working directories and preinstalled modules are
part of the deployment context. These motivate both short jobs and long-lived Actors; they do not
require each CoD Actor to occupy an OS subprocess.
[Repeated batches](https://github.com/complyue/sedh/blob/2567b52ed58985d0edfa6312a3b883c5cf8b79ab/host.py/sedh/demo/tt/__main__.py) ·
[Session workload](https://github.com/complyue/sedh/blob/2567b52ed58985d0edfa6312a3b883c5cf8b79ab/host.py/sedh/demo/session.py#L59-L130) ·
[Worker deployment](https://github.com/complyue/sedh/blob/2567b52ed58985d0edfa6312a3b883c5cf8b79ab/SmartSwarm/WorkerSetup.md#L1-L37)

### Reference inconsistencies are evidence limits

Current Python workers send `SettleResult`, and the Python HeadHunter exposes it as a settlement
callback; its old `result_ch.put` path is commented out. The Edh worker also sends `SettleResult`, but
the Edh HeadHunter does not declare that method. The Edh batch example returns `r` where the current
worker reads `res`. These paths need reconciliation before they can serve as runnable protocol
acceptance. They are reference-repository maintenance issues, not new CoD language decisions.
[Python settlement path](https://github.com/complyue/sedh/blob/2567b52ed58985d0edfa6312a3b883c5cf8b79ab/host.py/sedh/headhunter.py) ·
[Edh worker](https://github.com/complyue/sedh/blob/2567b52ed58985d0edfa6312a3b883c5cf8b79ab/edh_modules/swarm/worker/__main__.edh) ·
[Edh HeadHunter](https://github.com/complyue/sedh/blob/2567b52ed58985d0edfa6312a3b883c5cf8b79ab/edh_modules/swarm/headhunter.edh) ·
[Edh batch example](https://github.com/complyue/sedh/blob/2567b52ed58985d0edfa6312a3b883c5cf8b79ab/edh_modules/swarm/demo/batch.edh)

## Candidate CoD workload

Use a work-source Actor, a node-management Actor and two job-processing Actors, one local and
one reached through a network adapter. These are application roles. A client follows a job using
its job or attempt identity and a typed reply endpoint. Reusing the same physical thread or
process for another job gives the former client no authority over that job. Each Actor has its
own dedicated OS thread and dynamic context. A simulation or a live service can interpret the
same typed operation. This exercises the charter's program /
execution / environment separation.

The following names describe **application message families**, not selected CoD declarations or
builtin messages. Fields are design questions made concrete for review.

| Application family | Intended contents | Decision exposed by the workload |
| --- | --- | --- |
| Resource demand / offer | Request identity, required capabilities, offered capacity and expiry | Resource reservation is separate from execution; offers and acceptances may need repetition or expiry. |
| Job request | Business job identity, attempt identity, admitted operation, typed inputs and response route | A new execution attempt must be distinguishable from a retransmission of the same request. |
| Job outcome | Job/attempt identity and a typed success or domain-failure payload | A received outcome needs application-defined settlement and duplicate handling. |
| Session operation | Session identity, operation and response route | Stateful interactions need a lifetime and sequencing protocol above chan. |
| Cancellation request / outcome | Job/attempt identity and the precisely reported stopping stage | Request receipt, stopped computation and completed cleanup are different observations. |
| Optional reliable-flow message | Flow identity, sequence, acknowledgement or credit | Each application selects additional sequencing, reliability or flow regulation above transport backpressure. |

An outcome can use a domain sum to preserve alternative-specific payloads. Static typing must reject
an unrelated payload at a sending endpoint and constrain receiving code to the declared family.
The message-family type, regional representation, codec/schema version and endpoint authority remain
different checks. A version mismatch is not an invitation to execute a same-spelled remote function.

Request and response protocols should make their unknown state explicit. For example, after losing
a result the work source knows that no usable result has been observed; it does not know that the
attempt never ran. It may retry an idempotent operation, query a durable application record, wait,
or report uncertainty according to that job's policy. The language does not choose one policy for
all business operations.

## Ownership and execution constraints

### Messages carry admissible data and capabilities

An asynchronously retained message cannot contain a `here` body borrowing a sender activation or an
ordinary proc value. `here` remains bounded by synchronous use, and procedures are not storable
callable values. Sending a typed operation description lets the receiver invoke its own admitted
implementation. This must preserve the module interpretation and required dynamic bindings.
[Callable value kinds](CallableStorageAndHereAccess.md#callable-value-kinds) ·
[Explicit lifetime principle](GlobalVariablesDoneRight.md#explicit-lifetimes-without-closures)

Region-internal direct edges stay in one region. A return endpoint or external business identity
cannot be represented by an arbitrary direct pointer into another region. An intrinsic endpoint
carrier, or a checked ID and routing capability, needs a specific storage and transfer contract.
Readonly access alone neither freezes an entire graph nor grants a shared lifetime.
[Regional graph boundary](GraphGrainedMemoryManagement.md#relationships-between-regions) ·
[Independent access qualifiers](RegionalOwnership.md#independent-binding-and-target-qualifiers)

Each launch resets the dynamic context to its initial state. Module initialization and shutdown
ownership, publication of shared data, and native failure containment remain open. An Actor
must not acquire access to another Actor's live lexical state merely because both run in one OS
process. Native foreign entry must preserve the Actor's context and live owners.

### Exclusive regional handoff remains a candidate

For a packet carried by a typed chan, exclusive whole-region handoff is a candidate implementation of the local transport. Packet delivery has the same pool-independent contract at either endpoint. In one address
space a system owner can change without repairing internal graph edges. Remote transfer still has
byte transport, validation, compatibility and lifetime costs. Local release responsibility and
business-level execution identity are separate: releasing each local region exactly once does not
mean a job executes exactly once.

| Stage under the exclusive-handoff candidate | Required responsibility |
| --- | --- |
| Constructing the message | The sender holds the region and initializes the admitted root and graph. |
| Entering send / encountering a systemic fault | The exact consumption point and fault postcondition must say whether the source remains available, an intrinsic carrier returns ownership, or the runtime disposes of it. This is not selected yet. |
| Completed handoff | The sender's owning name is consumed and outbound sender access is revoked. The runtime holds the message until another holder receives it or it is discarded. |
| Ordinary loss or discard | The responsible holder releases the payload; the sender receives no individual delivery report and its old references do not revive. |
| Receiving and finishing | The receiving Actor obtains the agreed access and release responsibility; it releases the region or explicitly transfers it again. |

H-262 already fixes the consumed state of an ordinary source after a completed transfer. H-263 bounds
which old views remain usable under a new local holder; it does not revive outbound sender access.
H-170's returned-packet proposal has not selected the fault outcome. Owner-bearing carriers must
respect the intrinsic ownership boundary. Neither ordinary loss nor later receipt recreates a
sender-owned packet automatically.
[Post-send state](RegionalOwnership.md#post-send-state-without-implicit-null-dereferences) ·
[Holding and transfer](GraphGrainedMemoryManagement.md#actor-holding-and-transfer-review)

Message handoff also remains distinct from dynamic-resource replay. Capturing, hiding or restoring
an installed resource preserves its owner; it does not extract an owning packet. A receiving Actor
establishes the resources for its own work. Existing function permissions still apply: explicit-input
operations do not become procedural solely because they mutate memory or send a message, while a
function cannot use dynamic lookup or a proc call to bypass its permissions.
[Packet/resource distinction](GraphGrainedMemoryManagement.md#packet-transport-and-effect-resources-are-distinct-scenarios) ·
[Functions and explicit memory](SymbolicDynamicScope.md#function-region-state-access)

## Decisions to discuss next

The execution mapping, terminology, static message typing, absence of reception guarantees and the
system-fault feedback boundary are settled. The network path, region atomicity, queue/backpressure
support, endpoint separation and type-language prerequisite are also settled. The remaining questions should be evaluated against the
same workloads rather than reopening those decisions.

| Question | Candidate to examine first | Alternative or boundary that must be evaluated |
| --- | --- | --- |
| Who may receive from a chan? | One receiving Actor, with independently distributable sending capabilities; an Actor may use several typed chans. | Multiple receiving Actors require a defined selection and ownership rule. Neither form is chosen by the name chan. |
| How does a request carry a reply endpoint? | A typed sending capability with explicit lifetime/revocation behavior. | A checked logical route plus response type may be enough; carrier placement and network authority must be specified. |
| Who owns a message after a systemic send fault? | Compare the existing returned-owner proposal with a consuming-and-disposing operation and a pre-transfer rejection path. | Decide the exact transfer point, source availability, error surface and validity of prior views. No default is selected here. |
| What faults justify reporting? | Classify persistent local endpoint/route/service failures using concrete evidence and scope the report to the affected facility. | Define whether and how remote observations contribute, when a recovered facility becomes usable, and how transient loss remains fire-and-forget. No probability threshold is implied. |
| How does pressure reach the sender? | Budget queued regions, assembly, completed output and transport windows separately. | Decide when admission waits, expires or declines, how capacity resumes, and who owns each payload during a wait. |
| Who can close a chan, and what does close do? | A distinct owner/close capability; account for queued data and blocked receivers. | Specify late sends, drain/discard, shared collectors and the relationship to Actor termination. |
| How does an Actor stop? | Examine cooperative stopping and defined interruption points, with explicit cleanup and application cancellation messages. | Arbitrary native-thread termination, native faults and uninterruptible foreign calls need their own boundaries; sedh's subprocess SIGKILL is not that contract. |
| What happens if local thread creation fails? | Follow the owners passed to the entry procedure or named in the block's owner list when the child cannot enter. | Decide where failure is observed and which scope releases each resource. |
| How do Actor startup and shared modules interact? | Start with fresh TLS while preserving module semantic identity and established module lifetimes. | Decide module initialization, shared resources and final OS-process shutdown. |
| How are region boundaries recovered after a stream hole? | Compare region-aware framing over AKCP with a region envelope spanning existing records. | Decide how to identify transfers, detect a missing header/tail, validate complete coverage and root compatibility, and reclaim incomplete storage on expiry or epoch change. |

The one-receiver candidate follows the REPL and result-collector workloads particularly well. It is
a recommendation to investigate, not an approval of MPSC semantics or a chan API. Likewise,
cooperative cancellation is a candidate whose behavior must be reconciled with ordinary exceptions,
deferred cleanup and blocking/native operations before implementation review.

## Acceptance scenarios

These are design acceptance workloads, not a test suite or a claim of implemented behavior.

| Scenario | Observation required before the associated scope can be accepted |
| --- | --- |
| Both launch forms in one OS process | For each form, observe a dedicated child OS thread, fresh initial TLS and independent installations of the same symbol. The named entry procedure and the owner-list body can each use their transferred region and handle after the launcher leaves its scope. Check a mixed scalar argument, reject reuse of a consumed owner, and observe one release per resource still owned on normal or exceptional child exit. |
| Nested launch with only an independent scalar argument | The nested launch still requires an explicit target swarm. There is no resource handoff for the provider to perform for that argument. Ordinary launch admission still applies. |
| Named entry with a swarm region and an omitted scalar default | Run the caller-context default in the launcher. If it reads a valid local installation of `@amount` holding 7, the child receives the scalar 7 and starts with fresh initial TLS. If that default raises before entry, the launch has not consumed the prepared region. In the definition-context variant, the default runs in the child’s parameter preamble. If it raises after entry, child cleanup releases the transferred region even though the entry body never starts. [Parameter initialization order](SymbolicDynamicScope.md#named-arguments-defaults-and-complete-binding) · [Consuming calls](ExceptionControl.md#e4--consuming-calls-and-results) |
| Static payload mismatch | An incompatible message is rejected by the source type contract; an invalid external representation never becomes an admitted typed value. |
| A deliberately lost message | Ordinary send remains fire-and-forget; all retained payload storage is accounted for, without a synthesized receipt or sender-view revival. |
| A continuing communication fault | The selected fault policy reports the system condition to the sender; the ownership outcome is derivable and a later recovery has defined behavior. |
| A region larger than one AKCP record and one transport window | With sufficient assembly resources, a slow receiver can eventually obtain one complete valid region. Internal window credit can advance during assembly. |
| Loss in A followed by complete B | Repeat for the first, middle and final piece of A. A repaired in time is eligible whole. On abandonment A contributes no visible root or prefix. B remains independently deliverable. Late pieces or pieces from another epoch cannot revive A. |
| Receiver stops draining and later resumes | Pressure reaches admission or transmission within its budget. Resumption admits work as capacity frees. Check byte and region accounting through expiry, close and transfer failure. |
| A worker holds only the sending end of a result channel | A compatible result enters that end, and a collector can consume received results with only the receiving end. Reject a mismatched type or an operation requiring the other direction. Verify local pipeline feedback stays within its traversal. |
| A request carrying a reply route | The response type and sending authority are correct; the response endpoint can expire without retaining a caller frame. |
| Two workers sharing a collector | One worker's connection or Actor termination cannot close a collector owned by another party without the chosen authority. |
| A regional graph sent before sender scope exit | On the admitted delivery path, the receiver can use the graph after that scope ends; sharing/cycles survive and stale sender access rejects. |
| Loss after business effects but before an outcome arrives | The application distinguishes an unknown result from non-execution; its retry/settlement protocol defines duplicate effects. |
| Cancellation racing with a result | Application identities determine what each response means; no cancellation request alone proves cleanup or effect reversal. |
| Full queue, close and receiver exit | The selected policy accounts for queued messages, late senders and waiting receivers without leaks or accidental reliability promises. |
| Local and remote execution of one job description | Type and interpretation obligations hold in both contexts; physical transport and application reliability retain separate contracts. |

Start by closing the chan endpoint, ownership-on-fault and resource/close questions against a local
two-Actor trace. Then apply the same contracts to the REPL reply route and shared collector. The
cluster workload can add discovery, recruitment, attempt identities and settlement above that
boundary. This progression keeps the research grounded in nedh/sedh while preserving the CoD
decisions already made.
