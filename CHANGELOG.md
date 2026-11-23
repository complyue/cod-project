# Changelog

## 2026-11-23 — Distinguish published seed support from booter verification

<a id="k-native-callable-seed-status-20261123"></a>
**K-NATIVE-CALLABLE-SEED-STATUS-20261123.** D-20-C-DECLARATIONS interop r28 to r29 updates implementation status under H-NATIVE-CALLABLE-KIND-20261123 and H-NATIVE-BOOLEAN-RESULT-20261123. The published seed admits function linkage at `4540a4bdadf93bc2f81705e06d43a076fd1c4855:cod/lang_syntax.hh` and validates ordinary Boolean provider results at that commit’s `cod/native/provider_call.h`. The booter’s own checker, generators and bundled predicate have a separate delivery checkpoint. P-644/P-645 and the source contract retain their scope.

## 2026-11-23 — Confirm idiomatic guidance standards and canonical source style

<a id="h-idiomatics-guideline-20261123"></a>
**H-IDIOMATICS-GUIDELINE-20261123.** In the current cod project conversation on 2026-11-23 (America/Los_Angeles), the project user required all Idiomatic CoD content to follow settled language design independently of implementation progress, with genuine open design questions listed separately for discussion. They approved the proposed quality standards, requested an independent `docs/IdiomaticsGuideline.md` committed on the latest main, and kept the IdiomaticCoD rewrite for after the design discussions. The same approval made code style a hard requirement, set an uncompromising, almost configuration-free formatter direction, and recommended automatic formatting in commit hooks and formatting check gates. The subsequent message specified: “风格方面，最大列宽初步定 100”.

D-DOC-IDIOMATICS and D-10-FORMAT-POLICY qualify guideline r1 under P-694–P-697/C-06. The initial 100-column maximum applies to CoD source; remaining unconfirmed formatting details and tool implementation/deployment retain their own scope. Existing language decisions are preserved. The predecessor language and authoring baseline is `4a7b6bab67845ced1427888d6c32420d0a37d5fc:AGENTS.md` and the same commit's `design/SymbolicDynamicScope.md`. This record is an attributed decision paraphrase with the exact width statement; no stable locator or exact timestamp for this conversation is available.
