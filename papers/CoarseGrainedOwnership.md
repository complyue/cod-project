<a id="coarse-grained-ownership-for-relocatable-object-graphs"></a>

# Which references survive replacement of a region?

A CoD procedure can hold a reference into a region, replace the region in its owning variable,
and select a field with the same name and type again. Only the second selection can refer to the
replacement. The first selection still depends on the actual owner it acquired. A useful design
for graph editing must make this failure visible even if the replacement contains the same graph.

Consider an editor holding a selection while it constructs the next revision of a railway graph.
The paper asks exactly when reading the selection is lawful, and what the editor must do to select
the corresponding station in the new revision. The lifetime rule is settled. The design space
concerns how a CoD interface can express the dependency and how a tool can explain a rejected
edit.

## A replacement with one observable boundary

This small example specializes the [occupied-owner replacement rule](../design/RegionalOwnership.md#replacing-an-occupied-region-owner).
The single scalar makes it possible to check the ordering without any graph traversal.

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

Call the first actual owner **A** and the replacement **B**. These letters distinguish two
acquisitions in the argument. A program need not allocate identifiers with these names.

| Point in the procedure | What the program can establish |
| --- | --- |
| `prior` is bound | Its target is the field in A. Reading it into `saved` produces the independent scalar 7. |
| The right-hand side constructs B | A is still held. Reading `prior` to initialize B's field yields 7. |
| Construction and required cleanup succeed, and the owning result is delivered | The assignment displaces A and releases it once. The holder `r` now owns B. Any later use through `prior` would need A to remain live. |
| `current` is bound | This selection follows `r` into B and reads 8. The procedure can return 1. |

Changing the final line to `return current - prior;` must be rejected. Both spellings of
`in r` are identical in the source. Their declarations execute on opposite sides of delivery.
The first binding cannot be reinterpreted using the second one's actual owner.

There is a useful failure case. If construction fails before delivery and the right-hand side
only borrowed A, the holder still owns A on that failure path. A failure handler on that path can
use `prior` while A remains live. An operation that itself consumes A on entry has a different
contract. Failure after consuming entry cannot restore the caller's old owner. These cases
distinguish checking at the actual ownership boundary from simply forbidding all borrows after
the source line containing an assignment. [Owning-region rules](../design/RegionalOwnership.md#replacing-an-occupied-region-owner)

## Follow the selection through a graph edit

Suppose a revision contains outbound and inbound routes that both reach the same station;
a return link leads back to the outbound route. All direct edges, including the return link,
belong to that region's DCG. An editor reaches the station by both paths in revision A. It
retains two different reference bindings to one target. It also reads the station's domain key,
by value, to show the selection in its interface.

1. While A is held and accessible, reading the station along either path is admissible. Sharing
   the target and revisiting it through a cycle introduce no new owner.
2. The editor builds a separate revision B. B can contain a different station bearing the same
   key, an unchanged station represented at the same relative offset, or no matching station.
   Constructing B while A is live does not by itself invalidate either binding into A.
3. The editor delivers B into the occupied holder for A. This releases A. Both earlier bindings
   now fail the same ownership requirement, including one hidden inside an otherwise admissible
   callback. A callback that attempts the read later cannot use the earlier time at which it was
   registered as evidence.
4. If the editor wants to restore the selection, it performs its own lookup of the saved key in
   B. A unique match permits a fresh reference into B. If the edit deleted the station, the
   interface can clear the selection. The application's key and revision rules decide what counts
   as a match. They supply no access to the released region.

This example requires no direct edge between A and B. A tool can work with several live regions
through separate bindings. Where the business model relates them, it carries a typed key and
resolves it in an explicitly selected context. An edit published elsewhere likewise changes no
local borrow merely by being published. A reader still holding an independent, accessible old
revision can continue to inspect its own graph. The event that invalidated both selections in
step 3 was the release of their actual owner. [Regional graph and cross-region relationships](../design/GraphGrainedMemoryManagement.md#relationships-between-regions)

## Owner identity, target identity and representation

A view selects a target under an actual owner with particular access. Two views into one region
can select different targets or share a target while permitting different operations. Any proposed
editor signature or type synonym that names a view has to preserve all three facts when expanded.
Even identical bytes and an identical offset in a later revision cannot discharge the original
view's lifetime requirement. [Owner and target evidence](../design/RegionalAndLexicalStorageRefactor.md#shared-owner-information-from-checked-declarations)

Three similar-looking operations test whether an explanation has identified the right owner.

| Operation | Consequence for an earlier view |
| --- | --- |
| Transfer cleanup responsibility for the same live region into another local holder | The source owning name is consumed. The view may still be used where its original actual region remains live and accessible under the transfer rules. |
| Replace a region, including a consuming resize that returns a replacement | The old actual owner ends. The program must select a fresh view from the returned or newly installed owner. Equal addresses or preserved internal graph structure cannot justify an old use. |
| Copy a scalar from a field before release | A later read of the scalar uses the copy's own storage. This is why `saved` works in the example. |

A rule based on the spelling of the holder would get both of the first two rows wrong. A rule
based on address equality would get the second row wrong on any execution that reuses an
address. A rule based on graph reachability would confuse a property of edges *inside* B with the
authority to use a reference acquired in A. [Local same-region transfer](../design/RegionalOwnership.md#local-view-validity-after-ownership-transfer)
· [Consuming replacement](../design/RegionalAndLexicalStorageRefactor.md#explicit-resize-consumes-and-replaces-the-region)

## What a future checker and editor should explain

Two paper cases locate the harder design work. In the first, a conditional replaces the region
on just one path. A read of the original reference after the paths rejoin cannot be admitted
unless the original owner is known to remain live on every path reaching that read. Reading in
the branch where A survives, or freshly selecting through the occupied holder after the join,
gives the checker a different obligation. In the second, an editor registers a deferred action
while A is live. If the action can run after A is replaced, it cannot dereference its captured
view. Capturing a suitable key by value and looking it up under the then-selected revision
would implement a different, expressible operation. The author has to decide which revision
the action is intended to address.

For discussion, a diagnostic could show the selection that established the owner, the operation
that ended it on the relevant path, and the later attempted use. A graphical editor could mark
those three sites in its projection of the program. A wrapper that hides one of the sites is
a good next example: can its published interface express whether it borrows the original
region, hands on the same region, or returns a distinct one? Try each contract against the two
cases above. A false acceptance permits a stale access. A false rejection prevents a valid
same-owner handoff or a read during preparation of the replacement. The choice of interface
notation, diagnostic presentation and degree of inference remains open; the required outcomes
in these cases follow from the [ownership design](../design/RegionalOwnership.md#replacing-an-occupied-region-owner).
