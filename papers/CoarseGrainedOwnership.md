<a id="coarse-grained-ownership-for-relocatable-object-graphs"></a>
<a id="which-references-survive-replacement-of-a-region"></a>

# Choosing Which Allocations Retire Together

*Multi-owner tracking and address-space fragmentation in CoD*

A railway service runs through many timetable revisions. The engineer importing a revision knows when the decoded records can go away. The routing engineer knows when a search frontier can go away. The client decides how long to keep an itinerary. Those are three useful answers to “when can we reclaim this memory?” Each answer can cover thousands of allocations.

CoD lets an operation work across several such lifetimes. A region groups objects under one release responsibility. Every regional `ref` is bounded by its actual owner. The compiler must reject an escape or use that would let a `ref` outlive that owner. Holding a `ref` grants access during the owner's lifetime and gives its holder no authority to extend that lifetime. A call can read one owner's data, write another's and return a view of the result. This gives the application a choice about where to construct data that will survive. An allocator that places by owner can keep persistent catalog entries out of the span the importer will soon need back. The acquisition order alone says little about when the program will release each owner.

This paper makes two claims with different evidence. The ownership rules let a program retire a graph without scheduling a separate release for each node. The resulting placement **can** make a large contiguous buffer available to a long-running process. We give an exact allocation trace for the second claim, a case where grouping wastes space, and an experiment for finding out which outcome a real service gets.

## Start with the events that end a lifetime

Consider one import followed by many searches with overlapping lifetimes. Each search reads a specific timetable revision. Its caller wants an itinerary after the frontier has been discarded. The application can give the following groups distinct actual owners.

| Group | What belongs together | When its storage can be reclaimed |
| --- | --- | --- |
| Import workspace | Decoded records and temporary links needed to validate this revision | Validation has finished and the importer has built the durable index. |
| Timetable revision | Stations, routes, shared indexes and their internal links | The service retires this revision after its permitted readers have finished. Publication of its successor can happen earlier. |
| Search frontier | Candidates, scores and scratch paths for one search | The search finishes or unwinds. |
| Caller’s itinerary | Selected station keys, fares and the revision identifier needed to interpret them | The caller discards the result, possibly long after the search or revision ends. |

Releasing the frontier once retires its candidates and scratch paths. A shared station reached by many routes and a cycle in the route graph add no individual release decisions. One itinerary can survive many other searches, and a timetable can survive many itineraries. If the search owns one local region for both frontier and itinerary, it cannot return a `ref` to that itinerary across the region's retirement. To deliver the result, the program needs a destination with the caller's lifetime or an explicit transfer of the whole owner. Putting everything in a process-owned region would postpone reclamation until shutdown. The number of owners should follow the distinct retirement events the application actually needs.

Direct stored pointers connect objects within one regional graph. The itinerary can carry station keys by value and resolve them against an explicitly selected revision. It can also copy the display names needed on a historical receipt. The key survives release of the old storage. The application defines how to handle a renamed or deleted station. A live region can use typed pools to recycle slots earlier. Returning a slot to such a pool makes it available under the pool's rules. Releasing the region hands its backing to the responsible allocator. [One region per pointer graph](../design/GraphGrainedMemoryManagement.md#one-region-per-pointer-graph) · [Cross-region relationships](../design/GraphGrainedMemoryManagement.md#relationships-between-regions) · [Region ownership and library policy](../design/RegionalOwnership.md#intrinsic-region-ownership-versus-intra-region-library-policy)

## One call can use three owners and return a view into one

Suppose the caller has an existing best fare of 85. The frontier has found a fare of 80, and the timetable sets a ceiling of 90. The helper below records the improvement directly in the caller's result. Assume all three cells are initialized.

```cod
func keep_lower_fare(best:u64 in Result, candidate:const u64 in Search,
                     ceiling:const u64 in Timetable) -> ref u64 in Result
    where {const owner Result, const owner Search, const owner Timetable} {
    if (candidate <= ceiling) {
        if (candidate < best) { best <- candidate }
    }
    return best
}
```

The caller gets a view of its own cell, now holding 80. It can use the view after the frontier retires and, for this scalar result, after the timetable retires too. Returning `candidate` in place of `best` cannot satisfy this helper's `in Result` contract for independently acquired owners. Both cells happen to contain 80. Only the result cell can back this caller's returned reference after the search owner retires.

Each reference has one actual owner. The helper tracks three owner roles across the call. Repeating a binder would require the corresponding arguments to have the same actual owner. Distinct binders permit distinct owners and also admit a caller that legitimately supplies cells in the same one. In this workload the service acquires separate regions to obtain separate release points. The `const owner` declarations suffice because the helper allocates nothing. The `best` target is writable, so updating that existing cell is permitted. The caller retains responsibility for the result region. The project's cross-pool example exercises the related distinction by copying a scalar and returning either the source or the destination cell. [Owner-qualified calls](../design/RegionalOwnership.md#confirmed-owner-syntax) · [Owner access and target access](../design/RegionalOwnership.md#owner-constness-and-allocation) · [Cross-pool example](../design/RegionalOwnership.md#algorithms-across-existing-regions)

## How 1 GB free can fail a 10 MB request

The importer next needs a single, virtually contiguous 10 MB output buffer. Consider a backing allocator with a fixed 1,125 MB address window. It cannot grow, map the request elsewhere, or move live regions in this experiment. Use decimal units (1 GB = 1,000 MB), whole-MB aligned payloads and no header overhead. The two programs receive **the same domain allocations in the same order**. They differ in the region acquisitions made at the backing layer. Each region occupies one contiguous extent, and releasing its owner makes the entire extent immediately reusable in this model.

1. For each of 125 route groups, produce 8 MB of temporary decoded data, then 1 MB of catalog data. Validation needs all decoded data until the end of the import. The catalog must remain available afterwards. At peak, both programs occupy all 1,125 MB.
2. In the first program, each route group has its own 8 MB workspace region and 1 MB catalog region, acquired in that order from the backing allocator. At validation completion, release all 125 workspace owners and keep the 125 catalog owners. In the second, acquire one 1,000 MB workspace region and one 125 MB catalog region. Place the same domain data in their respective regions as it arrives. Release the single workspace owner at validation completion.
3. Request the 10 MB buffer and observe what this allocator can supply.

| Placement for those requests | Reusable space after validation | Largest available span | 10 MB request |
| --- | --- | --- | --- |
| Separate region pair per route group, acquired in arrival order | 125 holes × 8 MB = 1,000 MB | 8 MB | Fails |
| One workspace region and one catalog region for the import | One released workspace extent of 1,000 MB | 1,000 MB | Succeeds, leaving 990 MB in the extent |

In the first layout a whole live catalog region separates every adjacent pair of holes. Even an allocator that coalesces all neighboring free space has nothing to join. In the second, releasing one workspace owner returns a contiguous extent to the same backing allocator. The catalog stays readable in its own region. Both programs reclaim exclusively by retiring whole regions. Both have the same peak payload and free-byte total. The choice of domain boundaries changes the shape of the space available to the next request.

This is external fragmentation in the address space available to the request. An ordinary contiguous virtual buffer can map to scattered physical pages. A machine reporting 1 GB of free RAM therefore tells us little about the largest span in a fixed allocator window. Conversely, a process with room for another mapping might satisfy the request elsewhere. A failed request in this window becomes an application-level out-of-memory failure only when the application has no successful fallback. Distinguishing the largest free virtual span from total free bytes is also how Caldato and Stephens diagnose this failure mode in a managed runtime. [Virtual address spaces](https://learn.microsoft.com/en-us/windows-hardware/drivers/gettingstarted/virtual-address-spaces) · [Investigating memory issues](https://learn.microsoft.com/en-us/archive/msdn-magazine/2006/november/clr-inside-out-investigating-memory-issues)

## Which owner should carry an audit result?

The service now wants an audit summary after an import finishes. Let W be the locally owned workspace and A a separately acquired result region held by the caller. An attempted implementation returns a `ref` to the summary in W and exits the scope that owns W. **The compiler must reject that escape.** The summary has no independent lifetime. Storing the same reference in a callback or a record would preserve its dependency on W. The borrowed reference cannot keep W alive.

| What the audit requires | Structured ownership decision | What the caller may use |
| --- | --- | --- |
| Totals and station keys for a report | Construct the summary in A, or copy its eligible scalar fields into A while W is live. Release W at the end of the import. | A view into A, for as long as its actual owner remains live and accessible. |
| Inspection of the original decoded graph | Transfer ownership of the whole workspace through an owning result, or keep W under a holder whose lifetime spans the inspection. | Views into the original graph under that live owner. The holder is responsible for the whole region and its eventual release. |

These two interfaces give different costs. With a separately owned summary, the workspace can be reclaimed on schedule. With an owning graph result, its entire backing remains assigned to W until the holder releases it, however few fields the auditor reads. The old and new railway revisions likewise both require space while a replacement is constructed from a still-live original. A service sizing for this workload should budget for that overlap. In measurements of other region allocators, Berger, Zorn and McKinley found that keeping a region until the end of a phase can materially increase memory consumption. In CoD the design question is which owner the application chooses for each result. [*Reconsidering Custom Memory Allocation*](https://doi.org/10.1145/582419.582421)

Releasing an owner also has a backing-policy dimension. An allocator may cache a returned extent in a pool that cannot serve the requested size, or keep a reusable extent virtually mapped. Those policies affect the largest usable span and the process footprint differently. Inside a live region, typed slots can be recycled under the library's rules. The next whole-region request can use the space under the backing allocator's own rules when the region is released.

## A replacement with one observable boundary

Multi-owner tracking also protects the deliberate choice to retire a revision. In the following small instance the editor copies the old revision number and selects a fresh view after replacement. The existing revision remains readable while the next one is being constructed. [Occupied-owner replacement](../design/RegionalOwnership.md#replacing-an-occupied-region-owner)

```cod
region Revision { number:u64; }

proc advance() -> u64 {
    own region r:Revision <- make region Revision(4096u64 -> capacity) {
        number <- 7u64;
    };
    ref prior:u64 in r = r.number;
    val saved:u64 <- prior;

    r <- make region Revision(4096u64 -> capacity) {
        number <- prior + 1u64;
    };

    ref current:u64 in r = r.number;
    return current - saved;
}
```

The procedure returns 1. The read of `prior` while preparing the right-hand side sees 7 in the original owner. Successful delivery into `r` releases that owner once and installs the new owner with value 8. A later `return current - prior` would require a read from the released owner and must be rejected. If construction fails before delivery on a path that only borrowed the old region, the holder still owns it and the failure handler can read it. A consuming call that has already taken ownership has a different failure boundary.

## Follow the selection through a graph edit

Two route edges in revision A can point at the same station. Both views stay valid while A is held and accessible, including while the editor builds B. Releasing A ends both dependencies. If the editor saved the station key by value, it can look up that key in B, accept a fresh selection on a unique match, or clear the selection if the station was removed. The same key or relative byte offset in both revisions says nothing about the lifetime of the old view. Publishing B elsewhere can leave a reader's independently acquired snapshot of the older revision live until that reader releases it.

## Owner identity, target identity and representation

| Operation | What happens to the original dependency |
| --- | --- |
| Transfer responsibility for the same live region to another local holder | The source owning name is consumed. A view can remain usable where the same actual region is still live and accessible under the transfer rules. |
| Replace a region, including by a consuming resize | The old owner ends. Obtain new views from the replacement, even if the allocator reuses the numeric address. |
| Copy an eligible scalar into a different live owner | The destination cell has its own lifetime. Its bits can remain useful after the original target has gone. |

These distinctions allow a library to hide its graph layout while its caller can still tell which result survives. [Local same-region transfer](../design/RegionalOwnership.md#local-view-validity-after-ownership-transfer) · [Consuming replacement](../design/RegionalAndLexicalStorageRefactor.md#explicit-resize-consumes-and-replaces-the-region)

## What a future checker and editor should explain

When replacement occurs on one branch, a later use of the old view needs its owner to survive on every path reaching that use. A deferred action needs the same evidence when it actually runs. An editor can explain a rejection by showing the binding that selected the owner, the operation that ended it and the subsequent attempt to read. A developer can then decide whether the action requires the old revision, an independent copy, or a fresh lookup in the current revision.

## How to evaluate a long-running service

Replay the same sequence of imports, searches, overlapping readers and buffer requests with the same capacity budget. First run the fixed-window fixture above. Then compare the deployment allocator under its normal mapping and fallback policies with an implementation that steers allocations by domain owner. Include early reader completion, an audit with its own result owner, an audit that explicitly receives the whole graph owner, and a refresh that overlaps a still-live revision. Check that both implementations return the same itineraries. Separately check that returning a workspace `ref` beyond its owner's lifetime is rejected.

Record total reusable bytes **and** the largest aligned span usable by the requesting allocator just before each large request. Count failed requests and successful fallbacks. Also measure capacity held by explicitly live regions after the application stops using part of their contents, peak reserved and committed memory, resident set size and allocation/retirement latency over repeated revisions. At the application level, count the independent release decisions a feature actually requires. Thousands of graph nodes may add allocation work while adding only one decision about when their workspace retires.

The two layouts in the table can be reproduced by marking one cell per MB, assigning each cell its actual region, releasing the specified workspace owners as whole units, and counting adjacent free cells. That calculation checks the example's geometry. Evidence for a production CoD benefit must also show that the backing allocator honors the grouping, that the domain's retention events recur in real traces, and that fragmentation relief is worth any extra space held by long-lived owners.
