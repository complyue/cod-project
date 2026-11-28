# Actors and typed chan: network and cluster research

Design and research revision: 5 · 2026-11-28

## Design authority

At r5 the [resource pool direction](../CHANGELOG.md#h-go-resource-pools-20261128) requires an actual target pool for every go process and makes the pool a first-class ownership boundary. D-32-RESOURCE-POOLS/P-789 relates that boundary to both r4 launch forms and typed chan communication. The same-host shared-memory example examines feasibility. Pool construction, placement policy and endpoint syntax remain open for `e-wrks/cod`.

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
handle a partition. The local launch itself allocates one thread on that machine.
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

A **resource pool** is a first-class CoD object identifying the domain within which go processes can transfer resource ownership. A program should be able to pass a pool to a library and use that same pool for related work. Every go process launch targets a particular pool, even if it passes only scalars. A scheduling policy such as “use a subprocess” by itself does not identify a pool. A pool answers *where ownership can pass*. At any instant a particular holder still has the release obligation for each owned resource. Pool membership gives no general access to another member’s resources. A thread pool or subprocess pool supplies execution capacity. A region’s internal block pool supplies storage. Neither shared capacity nor shared storage establishes ownership-transfer authority. [Regional allocation policy](RegionalOwnership.md#intrinsic-region-ownership-versus-intra-region-library-policy)

Apply the same rule to an owning argument of a named entry procedure, a block’s explicit owner list, and an ownership-bearing message sent over a typed `chan`. Each receiving go process must be in the same actual pool for that handoff. Compare two independently created pools with identical configuration: the same-pool transfer is eligible, and the cross-pool transfer is invalid even if both workers share an OS process. Passing the same pool through a parameter or alias preserves its identity. Wrapping an owner in another message cannot change the boundary. Message type, ordinary owner/lifetime rules and the provider’s ability to transport that particular resource still apply.

For example, a producer in pool `P` builds a partition, then sends its region to an indexing worker. One candidate static signature relates the owned partition and the destination end of the chan through a common pool parameter `P`. A generic producer can then use either a thread worker or a subprocess worker when the backing works for both. An endpoint leading to a different pool `Q` fails the ownership check even if its message schema is identical. For a shared queue, the guarantee must cover every eligible receiver of an owning message. The way to express that relation and recover evidence after a dynamic pool selection is still a design question.

### A shared-memory region between local processes

An ordinary structural `Partition` region can in principle use OS shared memory as its backing. Consider one partition whose nodes refer to each other through region-relative locators. The parent finishes its writes. A local subprocess maps the same backing at its own base address and resolves the same root, shared nodes and cycles. The handoff must publish the completed writes before the receiving entry or chan consumer can use the region. Linux shared-memory objects and descriptor passing provide the native pieces for such a transport. The source root schema can stay the same. [Regional graph representation](GraphGrainedMemoryManagement.md#one-region-per-pointer-graph) · [POSIX shared memory on Linux](https://man7.org/linux/man-pages/man7/shm_overview.7.html) · [Unix-domain descriptor passing](https://man7.org/linux/man-pages/man7/unix.7.html)

Mapping the bytes and transferring the owner are separate events. After successful handoff the sender’s consumed binding and stale sender access paths cannot use the region, even if its native mapping still exists. The receiving side has the release obligation. A process-local native pointer or handle requires its own valid transport before the receiving program can use it. The mapping alone does not synchronize concurrent mutation or establish a lifetime for an external resource.

**Design probes**

| Case | Outcome or decision |
| --- | --- |
| Niladic entry or owner-free block with no target pool | Reject the launch. Having no ownership to check does not supply a target. |
| Same pool, different OS processes on one host, relocatable shared backing | Permit the ownership path. Verify a different mapping base, preserved graph edges, visible completed writes and one release responsibility. |
| Different pools, same address space, same region and message types | Reject the ownership path. Changing the backing or using a chan cannot authorize it. |
| Same pool, subprocess receiver, opaque handle usable only in the sender process | Pool identity meets one condition. The handle still lacks a usable subprocess transport. |
| Work is accepted into a queue and the receiving worker dies | The responsible queue or transport must account for the resource under the eventual failure contract. An ordinary chan send still gives no delivery receipt. |
| Two pools need the same business data | A candidate is to send self-contained data and construct a fresh region owned in the destination pool. Whether to offer such an operation and how to spell it remain open. It would transfer no existing owner across the boundary. |

One open placement question is whether a logical pool may span machines. Shared memory covers the local-host case. The network contract below already specifies whole-region delivery. A distributed-pool design would also have to establish common pool membership. Communication between distinct pools would have to respect their ownership boundary whichever transport they use. The placement syntax, pool startup and shutdown, queue acceptance and release after failure remain separate decisions. The booter and seed retain their existing scope.

## Regional transport over AKCP

The network transfer unit visible to the receiving Actor is one whole region. A sender may submit a
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

Use a work-source Actor, a node-management Actor and two worker Actors, with one worker local and
one reached through a network adapter. Each Actor has its own dedicated OS thread and dynamic
context. The same business operation can be interpreted by a simulation worker or a live-service
worker without changing the operation's typed description. This exercises the charter's program /
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

Exclusive whole-region handoff is a useful first candidate for a regional message. In one address
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
