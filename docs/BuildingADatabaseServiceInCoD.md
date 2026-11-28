# Building a Database Service in CoD — From Indexed DBMR Reads to DML Packets

*Design-stage application guide, draft 4.* Build a facsimile fulfillment service that answers “Which sheets make up edition E42?” and accepts “Reserve one complete set for order 100.” The first question needs a shared catalog. The second changes stock and creates an obligation to one customer. The exercise specifies expected observations for a local interpreter and a future Actor service. [Companion working paper](../papers/DSDS.md) · [Design authority and scope](../design/DesignAuthorityRegistry.md#d-dbmr-service-research)

## 1. Publish the catalog that both readers need

This catalog uses a disk-backed memory region (DBMR) so both applications can traverse one published index directly. Here DBMR describes how the catalog is stored. The package author chooses the product or provider that implements it. [Regional storage terminology](../design/GraphGrainedMemoryManagement.md#dbmr-as-a-storage-technique)

Define edition and sheet keys, constituent occurrences and the catalog root in a shared CoD vocabulary module. Supply the edition-key ordering to an index package and use it to build the B+ tree from key to edition. Give each edition an ordered sequence of occurrences. Each occurrence records its position and refers to its canonical sheet in the same region. For E42, put foldout F between plates A and B. Add a fixture in which F occurs twice. A distinct-sheet report must list F once. A packing list for that fixture must request two copies.

Publish catalog revision 17 and open it read-only in the fulfillment and editorial applications. Query through both openings and compare the ordered keys with the source records. Each application holds its own owner, even when both map the same file. Preserve the catalog revision and edition key on an order that needs later explanation. Resolve those keys under the selected opening. [DBMR publication and reopening](../design/ConstructionAndAcquisition.md#dbmr-modes) · [Shared catalog example](../papers/AlgebraicEffectsIsAllYouNeed.md#shared-ontology-packages-and-dbmr-databases)

## 2. Put the changing facts in the right place

| Data | Placement for this exercise |
| --- | --- |
| Catalog and an independently revised assembly manual | Two published, indexed, read-only DBMRs. Pin the revisions used for an order. |
| Stock, reservations and request outcomes | One small writable database under the service's transaction protocol. A reservation, its stock deductions and its successful request outcome commit together. |
| Incoming DML, execution scratch and outgoing result | Separate regions. Build the returned receipt or shortage in the response owner so it can survive disposal of the request and scratch. |

Start the local exercise with private writable working storage. The proposed advanced DBMR interface would let database servers reopen mutable backing and coordinate writers across processes. Its mapping, synchronization, flush and recovery contracts still need design and implementation. Document the failure model your service can actually support when offering durable receipts. [Update and history contracts](../design/DomainLanguageDesign.md#dl-06)

### Choose the packages and their maintainers

Keep the edition vocabulary and reservation rule in the domain package. Index maintenance, schema and packet compatibility, multiprocess concurrency control, and crash recovery can come from CoD packages maintained by other teams or a contracted database specialist. For this service, choose a transaction provider whose commit and recovery contract covers stock, reservations and request outcomes together. A specialist can supply and operate that whole component. Record which schema revisions its compatibility tools accept and which failures its durable receipt survives. When replacing or upgrading a provider, use the two-order race and lost-reply exercise below to accept the assembled service.

### Build the stock provider

In the planned package workflow, declare `region type StockDB` and its constructor and reservation methods in CoD. Only that exact nominal type matches the stock operations. The stock database has no CoD roots. The provider could open its stock index as a DBMR, or read stock rows from a journal into ordinary memory and commit updates through that journal. Clients call the same reservation method in either case. A helper that walks the catalog’s named roots can keep working directly on the catalog. [Nominal region types](../design/GraphGrainedMemoryManagement.md#opaque-nominal-region-types)

1. Generate the C header and adapter stubs from the declarations. Keep the handwritten stock provider in a separate C or C++ file that includes the generated header. For a C++ engine, implement the entry points declared with `extern "C"` in the header and translate any C++ exception at that boundary. For a provider with a mapped stock file and a journal, exercise a journal-open failure after mapping succeeds. Release the mapping and return the declared failure. The caller receives no database owner on that path.
2. Use the toolchain supplied by `codup`. The package build compiles the generated adapter as C and the engine in its source language, then links both with the CoD program and the target libraries the engine uses. Use `cod` to run the host exercise. Use `cfe` when delivering a saved executable for another supported target. Regenerating the header and stubs leaves the engine source intact. [Native package build plan](../applications/cod-booter/docs/CompilerPipeline.md#codup-toolchain-for-native-packages)
3. On a successful open, run the two-order race below through the reservation method. Drop the reply after commit and retry the same request. Check the returned receipt, the remaining stock and release of the acquired owner. A successful link checks the calling interface. The race and retry exercise the stock provider’s transaction promises.

## 3. Run one data manipulation program locally, then send it

Use ordinary CoD library operations to compose a data manipulation language (DML) program. The following is a plan written in words. The transportable value is a typed instruction graph.

1. Resolve E42 at catalog revision 17 and gather its required sheet counts.
2. In the stock transaction, check the request identity, then test all required counts. On success, deduct stock, create the reservation and record its outcome. A shortage leaves stock untouched.
3. Project a receipt with order, reservation and catalog revision, or a shortage with the missing sheet keys, into the result region.

Run this plan against a local database high-level virtual machine (HLVM) with fixed inputs. Once typed Actor transport is available, send the same plan and inputs in a request packet. The service runs the entry only after complete reception and validation. Its Actor supplies the authorized database bindings to the HLVM. Return the outcome in a separate packet. Use the shared vocabulary to inspect it directly. This native path requires no ORM or OGM. Keep the operation vocabulary, its versions and result projections explicit. [Packet lifecycle and compatibility](../design/DomainLanguageDesign.md#dl-04)

## 4. Exercise two different consistency contracts

Give revision 17 one occurrence each of A, F and B, with stock A=2, F=1, B=2. Race orders 100 and 101 in separate server processes against the same stock backing. For a service promising ACID reservations, require one committed reservation. After conflicts are resolved the other order reports F unavailable. Final stock is A=1, F=0, B=1. Readers entitled to committed stock must never observe half a reservation. Kill the successful writer after durable commit and before reply. Retrying the same request identity and payload must recover the recorded receipt without a second deduction. Record separately which process or storage failures the durability claim covers. [Commit and retry](../design/DomainLanguageDesign.md#dl-05)

Now disconnect a warehouse availability display before the sale. Permit the display to show F=1 and label it with the stock revision it has seen. Reservation requests still check authoritative stock. For this exercise, give the display the committed remaining quantity and stock revision. Apply a newer revision once and ignore older or duplicate snapshots. Restore communication, replay the committed change and repeat its delivery. With no further writes, the display should converge to F=0. This projection can accept delay and duplicate delivery while the service preserves the one-copy sales constraint. Record the lag, recovery work and bytes written per reservation as the read-only catalog grows.

Finally publish catalog revision 18 with a newly documented key sheet K. Keep revision 17 available. Read the original receipt after releasing the request and reopening the catalog. The old order must still explain A, F and B. A fresh order pinned to revision 18 must also check K. This separates the cost of an occasional catalog correction from the cost of each reservation.
