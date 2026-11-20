#!/usr/bin/env python3
"""Source checks for storage identity, access and control-flow facts."""
from pathlib import Path
import sys
import foundation_tests as runner

PRELUDE = 'using symbol "std:io" @print;\n'
SHADOW = PRELUDE + 'val x<-1u64; {{val x<-2u64; behave @print(x);}};'
PRODUCT = 'data Window {minimum:u64; maximum:u64;}'
runner.CASES = {
    "storage": (None, 0, "3\n4\n4\n3\n9\n3\n3\n2\n15\n7.5\n5\n6\n7\n8\n11\ntrue\n3\n12\n3\n2\n13\n", ""),
    "storage_shadow": (None, 0, "12\n11\n10\n", "[-Wshadow]"),
    "shadow_error": (SHADOW, 2, "", "treated as error"),
    "unrelated_warning_error": (SHADOW, 0, "2\n", "[-Wshadow]"),
    "uninitialized": ('val x<-make local u64; x;', 2, "", "before definite initialization"),
    "branch_initialization": ('proc f(flag:bool)->void { val x<-make local u64; if(flag){x<-1u64;{}}else{}; x; } f(false);', 2, "", "before definite initialization"),
    "conditional_declaration": ('proc f(flag:bool)->void { if(flag){val x<-1u64;}else{}; x; } f(false);', 2, "", "not available on every reaching path"),
    "short_circuit_initialization": ('val x<-make local u64; false && {x<-1u64;true}; x;', 2, "", "before definite initialization"),
    "short_circuit_declaration": ('false && {val x<-1u64;true}; x;', 2, "", "not available on every reaching path"),
    "const_storage": ('const val x<-1u64; x<-2u64;', 2, "", "readonly destination"),
    "const_target": ('val x:const u64<-1u64; x<-2u64;', 2, "", "readonly destination"),
    "const_parameter": ('func f(x:const u64)->const u64 {x<-2u64;return x;} f(1u64);', 2, "", "readonly destination"),
    "const_val_parameter": ('func f(const val x:u64)->const u64 {x<-2u64;return x;} f(1u64);', 2, "", "readonly destination"),
    "const_copy": (PRELUDE + 'const val x<-1u64; val copy<-x; copy<-2u64; behave @print(x); behave @print(copy);', 0, "1\n2\n", ""),
    "val_rebind": ('val x<-1u64,y<-2u64; x=y;', 2, "", "val storage has no reference binding"),
    "transfer_result_readonly": ('val x<-1u64; {x<-2u64}<-3u64;', 2, "", "readonly destination"),
    "full_scope_value_is_not_destination": ('{{ val x<-1u64; return x; }} <- 2u64;', 2, "", "assignment requires a storage destination"),
    "duplicate_group": ('val x<-1u64; {val x<-2u64;};', 2, "", "duplicate declaration"),
    "duplicate_branch": ('if(true){val x<-1u64;}else{val x<-2u64;};', 2, "", "duplicate declaration"),
    "duplicate_parameter": ('func f(x:u64)->const u64 {val x<-2u64;return x;} f(1u64);', 2, "", "duplicate declaration"),
    "initializer_lookup": ('val x<-x;', 2, "", "unknown scalar value"),
    "local_callable_shadow": ('func f()->void {} val f<-1u64; f();', 2, "", "nearest binding 'f' is not callable"),
    "local_symbol_shadow": (PRELUDE + 'val print<-1u64; behave @print(1u64);', 2, "", "nearest binding 'print' is not a symbol"),
    "narrow_store": ('val x:u8<-1u8; x<-256u64;', 2, "", "does not satisfy destination type"),
    "copy_type_mismatch": ('val x:u64<-true;', 2, "", "does not satisfy declared val type"),
    "generic_storage_error": ('func f(x:T)->const T where T {val y:T<-true;return y;} f(1u64);', 2, "", "does not satisfy declared val type"),
    "module_value_capture": ('val x<-1u64; func f()->const u64 {return x;} f();', 2, "", "module-local binding lookup and capture"),
    "module_symbol_shadow": (PRELUDE + 'val print<-1u64; proc f()->void {behave @print(1u64);} f();', 2, "", "module-local binding lookup and capture"),
    "lexical_product_initializer_order_and_borrow": (PRELUDE + PRODUCT +
        'proc mark(n:u64)->u64{behave @print(n);return n;}'
        'proc write(ref target:u64 in O) where {owner O}{defer (<-)(target,21u64);return;}'
        'ref w:Window in here = make Window {maximum<-mark(30u64);minimum<-mark(20u64);};'
        'ref selected=w.minimum;behave @print(w.maximum);write(w.minimum);'
        'behave @print(selected);behave @print((selected is w.minimum));',
        0, "30\n20\n30\n21\ntrue\n", ""),
    "lexical_product_distinct_subobjects": (PRELUDE + PRODUCT +
        'ref w=make Window in here {minimum<-7u64;maximum<-7u64;};'
        'behave @print((w is w.minimum));behave @print((w.minimum is w.minimum));'
        'behave @print((w.minimum is w.maximum));', 0, "false\ntrue\nfalse\n", ""),
    "lexical_product_partial_and_nested_initialization": (PRELUDE + PRODUCT +
        'data Outer {window:Window;}ptr p=make Outer;behave @print((p is not null));'
        'p.window.minimum<-20u64;behave @print(p.window.minimum);'
        'p.window.maximum<-30u64;ref ready=p;behave @print(ready.window.maximum);',
        0, "true\n20\n30\n", ""),
    "lexical_product_missing_field_at_finish": (
        PRODUCT + 'ref w=make Window {minimum<-20u64;};', 2, "", "missing initialization of field 'maximum'"),
    "lexical_product_uninitialized_field_read": (PRELUDE + PRODUCT +
        'ptr w=make Window;w.minimum<-20u64;behave @print(w.maximum);',
        2, "", "before definite initialization"),
    "lexical_product_incomplete_ref_after_branch": (PRODUCT +
        'proc check(flag:bool)->void{ptr w=make Window;w.minimum<-20u64;'
        'if(flag){w.maximum<-30u64;{}}else{};ref ready=w;return;}check(false);',
        2, "", "read of 'maximum' before definite initialization"),
    "lexical_product_repeated_outer_owner_allocation": (PRODUCT +
        'ptr w=make Window;for(i in 0u64..2u64){w=make Window;};',
        2, "", "requires a separate full lexical scope"),
    "lexical_product_readonly_projection": (PRODUCT +
        'ref w:const Window=make Window {minimum<-20u64;maximum<-30u64;};w.minimum<-21u64;',
        2, "", "readonly destination"),
    "lexical_product_null_before_projection": (PRELUDE + PRODUCT +
        'using "std:exceptions" NullPointerException;ptr w:Window=null in here;'
        'try{behave @print(w.minimum);}catch(e:const NullPointerException){behave @print(9u64);};',
        0, "9\n", ""),
    "borrowed_product_and_projected_results": (PRELUDE + PRODUCT +
        'data Policy {window:Window;}'
        'func identity(ref item:T in O)->ref T in O where {T,owner O}{return item;}'
        'func floor(ref rule:Policy in O)->ref u64 in O where {owner O}{return rule.window.minimum;}'
        'func select(ref rule:Policy in O)->ref u64 in O where {owner O}{return floor(identity(rule));}'
        'ptr policy=make Policy;policy.window.minimum<-20u64;policy.window.maximum<-30u64;'
        'ref current=identity(policy);ref retained=select(current);retained<-21u64;'
        'behave @print(policy.window.minimum);behave @print((retained is policy.window.minimum));'
        'behave @print((current is policy));', 0, "21\ntrue\ntrue\n", ""),
    "borrowed_product_defer_keeps_selected_input": (PRELUDE +
        'data Settings {enabled:bool;limit:u64;}'
        'proc configure(ref target:Settings in O) where {owner O}{target.enabled<-true;target.limit<-33u64;return;}'
        'proc run()->void{ref a=make Settings {enabled<-false;limit<-5u64;};'
        'ref b=make Settings {enabled<-false;limit<-6u64;};ptr current=a;'
        'defer{behave @print(a.enabled);behave @print(a.limit);behave @print(b.enabled);behave @print(b.limit);};'
        'defer configure(current);current=b;return;}run();', 0, "true\n33\nfalse\n6\n", ""),
    "borrowed_product_checks_complete_input": (PRODUCT +
        'proc borrow(ref target:Window in O) where {owner O}{return;}'
        'ptr window=make Window;window.minimum<-20u64;borrow(window);', 2, "", "read of 'maximum' before definite initialization"),
    "borrowed_product_write_updates_caller_boolean_proof": (
        'data Settings {enabled:bool;}'
        'proc arm(ref target:Settings in O) where {owner O}{target.enabled<-true;return;}'
        'ref state=make Settings {enabled<-false;};val unread<-make local u64;arm(state);'
        'if(state.enabled){unread;{}}else{};', 2, "", "read of 'unread' before definite initialization"),
    "const_product_owner_keeps_writable_target_access": (PRELUDE + PRODUCT +
        'proc update(ref target:Window in O) where {const owner O}{target.minimum<-7u64;return;}'
        'func inspect(target:const Window in O)->u64 where {const owner O}{return target.minimum;}'
        'ref window=make Window {minimum<-5u64;maximum<-6u64;};update(window);behave @print(inspect(window));',
        0, "7\n", ""),
    "const_product_owner_cannot_escalate_at_call": (PRODUCT +
        'proc update(ref target:Window in O) where {owner O}{target.minimum<-7u64;return;}'
        'proc appoint(ref target:Window in O) where {const owner O}{update(target);return;}'
        'ref window=make Window {minimum<-5u64;maximum<-6u64;};appoint(window);',
        2, "", "const owner cannot supply allocation access"),
    "readonly_product_parameters_require_const_owner": (PRODUCT +
        'func inspect(target:const Window in O)->u64 where {owner O}{return target.minimum;}',
        2, "", "readonly product inputs require 'const owner O'"),
    "nullable_product_argument_reads_only_a_present_field": (PRELUDE + PRODUCT +
        'func inspect(ptr window:const Window in O)->u64 where {const owner O}{'
        'if(window is null){return 9u64;}else{};return window.minimum;}'
        'ptr absent:Window=null in here;ptr reserved=make Window;reserved.minimum<-7u64;'
        'behave @print(inspect(absent));behave @print(inspect(reserved));', 0, "9\n7\n", ""),
    "nullable_product_argument_requires_the_read_field": (PRODUCT +
        'func inspect(ptr window:const Window in O)->u64 where {const owner O}{'
        'if(window is null){return 9u64;}else{};return window.maximum;}'
        'ptr reserved=make Window;reserved.minimum<-7u64;inspect(reserved);',
        2, "", "for deferred or borrowed read"),
    "nullable_product_result_preserves_the_projected_target": (PRELUDE + PRODUCT +
        'func floor(ptr window:Window in O)->ptr u64 in O where {owner O}{'
        'if(window is null){return null in O;}else{};return window.minimum;}'
        'ptr absent:Window=null in here;ptr present=make Window {minimum<-3u64;maximum<-11u64;};'
        'ptr missing=floor(absent);ptr minimum=floor(present);'
        'behave @print((missing is null));behave @print((minimum is present.minimum));', 0, "true\ntrue\n", ""),
    "nullable_product_call_preserves_partial_initialization": (PRELUDE + PRODUCT +
        'proc establish(ptr window:Window in O) where {owner O}{'
        'if(window is null){return;}else{};window.minimum<-17u64;return;}'
        'ptr reserved=make Window;establish(reserved);behave @print(reserved.minimum);', 0, "17\n", ""),
    "nullable_product_defer_retains_its_selected_target": (PRELUDE +
        'using "std:exceptions" OutOfMemoryError;data Counter {count:u64;}'
        'proc establish(ptr target:Counter in O) where {owner O}{'
        'if(target is null){return;}else{};target.count<-27u64;return;}'
        'proc run()->void{ptr reserved=make Counter;ptr selection=reserved;'
        'defer{behave @print(reserved.count);}catch(e:const OutOfMemoryError){return;};'
        'defer establish(selection);selection=null;return;}run();', 0, "27\n", ""),
    "nullable_product_failed_call_does_not_initialize_later_fields": (
        'using "std:exceptions" NullPointerException;data Counter {count:u64;}'
        'proc establish(ptr first:Counter in O,ptr second:Counter in O) throws NullPointerException where {owner O}{'
        'first.count<-1u64;second.count<-2u64;return;}'
        'ptr absent:Counter=null in here;ptr reserved=make Counter;'
        'try{establish(absent,reserved);}catch(e:const NullPointerException){};reserved.count;',
        2, "", "read of 'count' before definite initialization"),
    "product_call_initializes_the_dynamically_selected_field": (PRELUDE +
        'data Counter {count:u64;}'
        'proc establish(ptr target:Counter in O) where {owner O}{'
        'if(target is null){return;}else{};target.count<-17u64;return;}'
        'proc run(flag:bool)->void{ptr first=make Counter;ptr second=make Counter;'
        'ptr chosen=if(flag){first}else{second};establish(chosen);behave @print(chosen.count);return;}'
        'run(true);run(false);', 0, "17\n17\n", ""),
    "product_field_proof_does_not_follow_pointer_rebinding": (
        'data Counter {count:u64;}'
        'proc establish(ptr target:Counter in O) where {owner O}{'
        'if(target is null){return;}else{};target.count<-17u64;return;}'
        'proc run(flag:bool)->void{ptr first=make Counter;ptr second=make Counter;'
        'ptr chosen=if(flag){first}else{second};establish(chosen);chosen=second;chosen.count;return;}run(false);',
        2, "", "read of 'count' before definite initialization"),
    "lexical_product_embedded_binding_and_explicit_place_alias": (PRELUDE +
        'data Entry {tag:u64;ptr selected:u64;}val first<-17u64;val second<-21u64;'
        'ref item=make Entry {tag<-3u64;selected=first;};ptr var field->item.selected;'
        'field=second;item.selected<-23u64;behave @print(first);behave @print(second);', 0, "17\n23\n", ""),
    "lexical_product_nullable_edges_close_a_cycle_after_reservation": (PRELUDE +
        'data Node {count:u64;ptr next:Node;}ptr a=make Node;ptr b=make Node;a.count<-13u64;b.count<-19u64;'
        'a.next=b;b.next=a;ref ready=a;behave @print(ready.next.count);behave @print((ready.next.next is ready));',
        0, "19\ntrue\n", ""),
    "lexical_product_null_field_has_the_containing_owner": (PRELUDE +
        'data Entry {ptr selected:u64;}ref item=make Entry {selected=null;};'
        'behave @print((item.selected is null));val target<-43u64;item.selected=target;'
        'behave @print((item.selected is target));', 0, "true\ntrue\n", ""),
    "lexical_product_reference_field_rejects_another_owner": (
        'data Entry {ptr selected:u64;}val target<-3u64;ptr source=target;{{ref item=make Entry {selected=source;};}};',
        2, "", "must share the containing product's owner"),
    "lexical_product_slot_before_initialization_rejects_observation": (
        'data Entry {ptr selected:u64;}ptr item=make Entry;(item.selected is null);',
        2, "", "read of 'selected' before definite initialization"),
    "lexical_product_ptr_slot_does_not_initialize_its_target": (
        'data Entry {ptr selected:u64;}val reserved<-make local u64;'
        'ref item=make Entry {selected=reserved;};item.selected;',
        2, "", "read of 'selected' before definite initialization"),
    "lexical_product_ref_slot_requires_a_ready_target": (
        'data Entry {ref selected:u64;}val reserved<-make local u64;ref item=make Entry {selected=reserved;};',
        2, "", "read of 'ref field target' before definite initialization"),
    "lexical_product_initializes_the_dynamically_selected_slot": (PRELUDE +
        'data Entry {ptr selected:u64;}proc run(flag:bool)->void{val source<-13u64;'
        'ptr a=make Entry;ptr b=make Entry;ptr chosen=if(flag){a}else{b};'
        'chosen.selected=source;behave @print(chosen.selected);return;}run(true);run(false);', 0, "13\n13\n", ""),
    "lexical_product_does_not_initialize_an_unselected_slot": (PRELUDE +
        'data Entry {ptr selected:u64;}proc run(flag:bool)->void{val source<-13u64;'
        'ptr a=make Entry;ptr b=make Entry;ptr chosen=if(flag){a}else{b};'
        'chosen.selected=source;behave @print(a.selected);return;}run(false);',
        2, "", "read of 'selected' before definite initialization"),
    "lexical_product_initializes_a_slot_during_deferred_invocation": (PRELUDE +
        'using "std:exceptions" OutOfMemoryError;data Entry {ptr selected:u64;}'
        'proc run()->void{val source<-13u64;ptr item=make Entry;'
        'defer{behave @print(item.selected);}catch(e:const OutOfMemoryError){return;};'
        'defer (=)(item.selected,source);return;}run();', 0, "13\n", ""),
    "required_reference_component_becomes_readable_after_its_last_edge": (PRELUDE +
        'data Node {count:u64;ref next:Node;}ptr a=make Node;ptr b=make Node;'
        'a.count<-13u64;b.count<-19u64;a.next=b;b.next=a;ref ready=a;'
        'behave @print(ready.next.count);behave @print((ready.next.next is ready));', 0, "19\ntrue\n", ""),
    "required_reference_component_rejects_missing_reachable_payload": (
        'data Node {count:u64;ref next:Node;}ptr a=make Node;ptr b=make Node;'
        'a.count<-13u64;a.next=b;b.next=a;ref ready=a;',
        2, "", "read of 'count' before definite initialization"),
    "required_reference_component_rejects_missing_reachable_slot": (
        'data Node {count:u64;ref next:Node;}ptr a=make Node;ptr b=make Node;'
        'a.count<-13u64;b.count<-19u64;a.next=b;ref ready=a;',
        2, "", "read of 'next' before definite initialization"),
    "required_component_checks_later_rebinding_of_a_readable_object": (
        'data Node {count:u64;ref next:Node;}ptr a=make Node;ptr b=make Node;ptr loose=make Node;'
        'a.count<-13u64;b.count<-19u64;a.next=b;b.next=a;ref ready=a;ready.next=loose;',
        2, "", "before definite initialization"),
    "required_reference_component_completed_on_both_reaching_paths": (PRELUDE +
        'data Node {count:u64;ref next:Node;}proc run(flag:bool)->void{'
        'ptr a=make Node;ptr b=make Node;a.count<-13u64;b.count<-19u64;a.next=b;'
        'if(flag){b.next=a;{}}else{b.next=b;{}};ref ready=a;'
        'behave @print(ready.next.next.count);return;}run(true);run(false);', 0, "13\n19\n", ""),
    "required_reference_component_checks_a_scalar_referent_at_publication": (
        'data Entry {ref selected:u64;}val target<-make local u64;'
        'ptr item=make Entry;item.selected=target;ref ready=item;',
        2, "", "read of 'selected target' before definite initialization"),
    "borrowed_product_retains_the_required_referent_and_owner": (PRELUDE +
        'data Box {ref value:u64;}'
        'func borrow(ref source:const Box in O)->ref const u64 in O where {const owner O}{return source.value;}'
        'val target<-7u64;ref item=make Box {value=target;};ref result=borrow(item);'
        'behave @print((result is target));behave @print(result);', 0, "true\n7\n", ""),
    "borrowed_product_checks_container_field_and_payload_separately": (PRELUDE +
        'data Box {ptr value:u64;}func read(ptr source:const Box in O)->u64 where {const owner O}{'
        'if(source is null){return 1u64;}else{};if(source.value is null){return 2u64;}else{};return source.value;}'
        'val target<-7u64;ptr absent:Box=null in here;ref empty=make Box {value=null;};ref present=make Box {value=target;};'
        'behave @print(read(absent));behave @print(read(empty));behave @print(read(present));', 0, "1\n2\n7\n", ""),
    "borrowed_product_slot_readiness_does_not_initialize_returned_target": (
        'data Box {ptr value:u64;}func project(ref source:Box in O)->ptr u64 in O where {owner O}{return source.value;}'
        'val target<-make local u64;ref item=make Box {value=target;};ptr selected=project(item);selected;',
        2, "", "before definite initialization"),
    "borrowed_product_call_initializes_the_selected_referent": (PRELUDE +
        'data Box {ptr value:u64;}proc write(ref source:Box in O) where {owner O}{'
        'if(source.value is null){return;}else{};source.value<-19u64;return;}'
        'val target<-make local u64;ref item=make Box {value=target;};write(item);behave @print(target);', 0, "19\n", ""),
    "borrowed_product_recursion_can_return_a_whole_selected_node": (PRELUDE +
        'data Node {count:u64;ptr next:Node;}func next_of(node:const Node in O)->ptr const Node in O where {const owner O}{return node.next;}'
        'ref last=make Node {count<-11u64;next=null;};ref first=make Node {count<-7u64;next=last;};'
        'ptr following=next_of(first);ptr end=next_of(last);behave @print((following is last));'
        'behave @print((end is null));behave @print(following.count);', 0, "true\ntrue\n11\n", ""),
    "borrowed_product_projects_an_acyclic_product_referent": (PRELUDE +
        'data Leaf {count:u64;ptr optional:u64;}data Root {ref child:Leaf;}'
        'func count(ref source:const Root in O)->u64 where {const owner O}{return source.child.count;}'
        'ref leaf=make Leaf {count<-17u64;optional=null;};ref root=make Root {child=leaf;};behave @print(count(root));',
        0, "17\n", ""),
    "borrowed_product_indirect_and_inline_paths_can_share_a_target": (
        'data Leaf {bit:bool;}data Root {inline:Leaf;ref through:Leaf;}'
        'proc inspect(ref source:Root in O) where {owner O}{source.inline.bit<-false;source.through.bit<-true;'
        'val missing<-make local u64;if(source.inline.bit){missing;{}}else{};return;}'
        'ptr root=make Root;root.inline.bit<-false;root.through=root.inline;ref ready=root;inspect(ready);',
        2, "", "before definite initialization"),
    "borrowed_product_call_resolves_the_field_after_operand_preparation": (PRELUDE +
        'data Box {ref selected:u64;}func value(ref source:const Box in O,ignored:u64)->u64 where {const owner O}{return source.selected;}'
        'val first<-7u64;val second<-11u64;ref item=make Box {selected=first;};'
        'behave @print(value(item,{item.selected=second;0u64}));', 0, "11\n", ""),
    "deferred_product_call_reads_the_invocation_time_field_selection": (PRELUDE +
        'data Box {ref value:u64;}proc read(ref source:const Box in O) where {const owner O}{behave @print(source.value);return;}'
        'proc run()->void{val target<-7u64;val other<-11u64;ref item=make Box {value=target;};'
        'defer read(item);item.value=other;return;}run();', 0, "11\n", ""),
    "borrowed_product_rebinding_changes_the_caller_slot": (PRELUDE +
        'data Box {ptr value:u64;}proc update(ref source:Box in O,ref target:u64 in O) where {owner O}{source.value=target;return;}'
        'val first<-7u64;val second<-11u64;ref item=make Box {value=first;};update(item,second);'
        'behave @print(item.value);behave @print((item.value is second));', 0, "11\ntrue\n", ""),
    "borrowed_product_slot_write_survives_exceptional_completion": (PRELUDE +
        'using "std:exceptions" NullPointerException;data Box {ptr selected:u64;}'
        'proc replace(ref entry:Box in O,ref replacement:u64 in O,ptr missing:u64 in O) throws NullPointerException where {owner O}{'
        'entry.selected=replacement;missing;return;}'
        'val first<-7u64;val second<-11u64;ptr absent:u64=null in here;ref item=make Box {selected=first;};'
        'try{replace(item,second,absent);}catch(e:const NullPointerException){behave @print(item.selected);};',
        0, "11\n", ""),
    "borrowed_product_paths_can_rebind_the_same_runtime_slot": (PRELUDE +
        'data Box {ref selected:u64;}func replace(ref left:Box in O,ref right:Box in O,ref replacement:u64 in O)->u64 where {owner O}{'
        'left.selected=replacement;return right.selected;}'
        'val first<-7u64;val second<-11u64;ref item=make Box {selected=first;};'
        'behave @print(replace(item,item,second));behave @print(item.selected);', 0, "11\n11\n", ""),
    "borrowed_slot_rebinding_invalidates_the_old_selection_on_overlapping_paths": (
        'data Box {ref selected:bool;}proc replace(ref left:Box in O,ref right:Box in O,ref replacement:bool in O) where {owner O}{'
        'right.selected<-false;replacement<-true;left.selected=replacement;val missing<-make local u64;'
        'if(right.selected){missing;{}}else{};return;}'
        'val first<-false;val second<-true;ref item=make Box {selected=first;};replace(item,item,second);',
        2, "", "before definite initialization"),
    "borrowed_product_rebinding_does_not_initialize_the_new_referent": (
        'data Box {ptr selected:u64;}proc replace(ref item:Box in O,ptr replacement:u64 in O) where {owner O}{'
        'item.selected=replacement;return;}val first<-7u64;val incomplete<-make local u64;'
        'ref item=make Box {selected=first;};replace(item,incomplete);item.selected;',
        2, "", "before definite initialization"),
    "nullable_product_argument_may_skip_a_slot_write": (
        'data Box {ptr selected:u64;}proc replace(ptr entry:Box in O,ref replacement:u64 in O) where {owner O}{'
        'if(entry is null){return;}else{};entry.selected=replacement;return;}'
        'proc run(flag:bool)->void{val incomplete<-make local u64;val second<-11u64;ref item=make Box {selected=incomplete;};'
        'ptr conditional=if(flag){item}else{null in here};replace(conditional,second);item.selected;return;}run(false);',
        2, "", "before definite initialization"),
    "direct_call_initializes_a_reserved_embedded_slot": (PRELUDE +
        'data Box {ptr selected:u64;}proc replace(ptr entry:Box in O,ref replacement:u64 in O) where {owner O}{'
        'if(entry is null){return;}else{};entry.selected=replacement;return;}'
        'val second<-11u64;ptr item=make Box;replace(item,second);behave @print(item.selected);', 0, "11\n", ""),
    "slot_update_through_nested_product_keeps_preexisting_view": (PRELUDE +
        'data Box {ref selected:u64;}data Root {ref child:Box;}'
        'proc replace(ref entry:Root in O,ref replacement:u64 in O) where {owner O}{entry.child.selected=replacement;return;}'
        'val first<-7u64;val second<-11u64;ref item=make Box {selected=first;};ref saved=item.selected;'
        'ref outer=make Root {child=item;};replace(outer,second);behave @print(saved);behave @print(item.selected);',
        0, "7\n11\n", ""),
    "deferred_product_call_rebinds_saved_container_with_saved_replacement": (PRELUDE +
        'data Box {ref selected:u64;}'
        'proc replace(ref box:Box in O,ref replacement:u64 in O) where {owner O}{box.selected=replacement;return;}'
        'proc run()->void{val first<-7u64;val second<-11u64;val third<-13u64;'
        'ref a=make Box {selected=first;};ref b=make Box {selected=third;};ref destination=a;ref argument=second;'
        'defer{behave @print(a.selected);behave @print(b.selected);};defer replace(destination,argument);'
        'destination=b;argument=third;return;}run();', 0, "11\n13\n", ""),
    "deferred_product_assignment_uses_source_slot_at_invocation": (PRELUDE +
        'data Box {ref source:u64;ref destination:u64;}'
        'proc set_target(ref entry:Box in O) where {owner O}{entry.destination=entry.source;return;}'
        'proc run()->void{val first<-7u64;val second<-11u64;val third<-13u64;'
        'ref item=make Box {source=first;destination=third;};defer{behave @print(item.destination);};'
        'defer set_target(item);item.source=second;return;}run();', 0, "11\n", ""),
    "deferred_product_read_checks_the_new_referent": (PRELUDE +
        'data Box {ptr selected:u64;}proc read(ref entry:const Box in O) where {const owner O}{'
        'if(entry.selected is null){return;}else{};behave @print(entry.selected);return;}'
        'proc run()->void{val first<-7u64;val missing<-make local u64;ref item=make Box {selected=first;};'
        'defer read(item);item.selected=missing;return;}run();',
        2, "", "initialization"),
    "deferred_product_read_skips_the_old_unready_target_when_the_slot_is_null": (PRELUDE +
        'data Box {ptr selected:u64;}proc read(ref entry:const Box in O) where {const owner O}{'
        'if(entry.selected is null){behave @print(5u64);return;}else{};behave @print(entry.selected);return;}'
        'proc run()->void{val missing<-make local u64;ref item=make Box {selected=missing;};'
        'defer read(item);item.selected=null;return;}run();', 0, "5\n", ""),
    "deferred_product_read_observes_an_earlier_running_cleanup": (PRELUDE +
        'data Box {ref selected:u64;}proc read(ref entry:const Box in O) where {const owner O}{behave @print(entry.selected);return;}'
        'proc run()->void{val first<-7u64;val second<-11u64;ref item=make Box {selected=first;};'
        'defer read(item);defer{item.selected=second;};return;}run();', 0, "11\n", ""),
    "deferred_product_call_initializes_the_late_referent_only": (PRELUDE +
        'using "std:exceptions" OutOfMemoryError;data Box {ptr selected:u64;}'
        'proc fill(ref entry:Box in O) where {owner O}{if(entry.selected is null){return;}else{};entry.selected<-19u64;return;}'
        'proc run()->void{val first<-7u64;val later<-make local u64;ref item=make Box {selected=first;};'
        'defer{behave @print(first);behave @print(later);}catch(e:const OutOfMemoryError){return;};'
        'defer fill(item);item.selected=later;return;}run();', 0, "7\n19\n", ""),
    "deferred_product_write_does_not_initialize_the_saved_reference": (PRELUDE +
        'data Box {ptr selected:u64;}'
        'proc fill_and_read(ref entry:Box in O,ptr observed:u64 in O) where {owner O}{'
        'if(entry.selected is not null){entry.selected<-19u64;{}}else{};'
        'if(observed is not null){behave @print(observed);}else{};return;}'
        'proc run()->void{val missing<-make local u64;val replacement<-7u64;ref item=make Box {selected=missing;};'
        'defer fill_and_read(item,missing);item.selected=replacement;return;}run();', 2, "", "initialization"),
    "deferred_product_read_retains_the_selected_container": (PRELUDE +
        'data Box {ref value:u64;}proc print_value(ref entry:const Box in O) where {const owner O}{behave @print(entry.value);return;}'
        'proc run(flag:bool)->void{val first<-7u64;val second<-11u64;val final_a<-19u64;val final_b<-23u64;'
        'ref a=make Box {value=first;};ref b=make Box {value=second;};ptr chosen=if(flag){a}else{b};'
        'defer print_value(chosen);chosen=b;a.value=final_a;b.value=final_b;return;}run(true);run(false);',
        0, "19\n23\n", ""),
    "deferred_product_rebinding_writes_only_the_saved_container": (PRELUDE +
        'data Box {ref value:u64;}proc change(ref entry:Box in O,ref replacement:u64 in O) where {owner O}{entry.value=replacement;return;}'
        'proc run(flag:bool)->void{val first<-7u64;val second<-11u64;val third<-19u64;'
        'ref a=make Box {value=first;};ref b=make Box {value=second;};ptr chosen=if(flag){a}else{b};'
        'defer{behave @print(a.value);behave @print(b.value);};defer change(chosen,third);chosen=b;return;}run(true);run(false);',
        0, "19\n11\n7\n19\n", ""),
    "deferred_product_rebinding_follows_a_changed_ancestor": (PRELUDE +
        'data Leaf {ref value:u64;}data Box {ref child:Leaf;}'
        'proc change(ref entry:Box in O,ref replacement:u64 in O) where {owner O}{entry.child.value=replacement;return;}'
        'proc run()->void{val first<-7u64;val second<-11u64;val third<-19u64;'
        'ref a=make Leaf {value=first;};ref b=make Leaf {value=second;};ref item=make Box {child=a;};'
        'defer{behave @print(a.value);behave @print(b.value);};defer change(item,third);item.child=b;return;}run();',
        0, "7\n19\n", ""),
    "deferred_product_read_requires_the_selected_container_field_to_be_readable": (PRELUDE +
        'data Box {ptr value:u64;}proc print_value(ref entry:const Box in O) where {const owner O}{'
        'if(entry.value is null){return;}else{};behave @print(entry.value);return;}'
        'proc run(flag:bool)->void{val first<-7u64;val second<-11u64;val missing<-make local u64;'
        'ref a=make Box {value=first;};ref b=make Box {value=missing;};ptr chosen=if(flag){a}else{b};'
        'defer print_value(chosen);chosen=a;a.value=second;return;}run(false);', 2, "", "initialization"),
    "deferred_product_rebinding_does_not_initialize_another_candidate_slot": (PRELUDE +
        'data Box {ptr value:u64;}proc change(ptr entry:Box in O,ref replacement:u64 in O) where {owner O}{'
        'if(entry is not null){entry.value=replacement;{}}else{};return;}'
        'proc run(flag:bool)->void{val first<-7u64;ptr a=make Box {value=null;};ptr b=make Box;'
        'ptr chosen=if(flag){a}else{b};defer{behave @print((b.value is null));};defer change(chosen,first);return;}run(true);',
        2, "", "initialization"),
    "deferred_product_read_requires_only_the_invocation_time_ancestor": (PRELUDE +
        'data Leaf {value:u64;}data Box {ptr child:Leaf;}'
        'proc inspect(ref entry:const Box in O) where {const owner O}{'
        'if(entry.child is null){return;}else{};behave @print(entry.child.value);return;}'
        'proc run()->void{ptr first=make Leaf;ref second=make Leaf {value<-19u64;};ref item=make Box {child=first;};'
        'defer inspect(item);item.child=second;return;}run();', 0, "19\n", ""),
    "deferred_product_read_rejects_the_invocation_time_unready_ancestor": (PRELUDE +
        'data Leaf {value:u64;}data Box {ptr child:Leaf;}'
        'proc inspect(ref entry:const Box in O) where {const owner O}{'
        'if(entry.child is null){return;}else{};behave @print(entry.child.value);return;}'
        'proc run()->void{ref first=make Leaf {value<-7u64;};ptr second=make Leaf;ref item=make Box {child=first;};'
        'defer inspect(item);defer{item.child=second;};return;}run();', 2, "", "initialization"),
    "deferred_product_reader_uses_the_live_ancestor_after_later_initialization": (PRELUDE +
        'using "std:exceptions" NullPointerException;data Leaf {value:u64;}data Box {ref child:Leaf;}'
        'func prepare(flag:bool)->u64 throws NullPointerException{'
        'if(flag){ptr absent:u64=null in here;return absent;}else{};return 19u64;}'
        'proc inspect(ref entry:const Box in O) where {const owner O}{behave @print(entry.child.value);return;}'
        'proc run(flag:bool)->void throws NullPointerException{ref first=make Leaf {value<-7u64;};'
        'ref item=make Box {child=first;};defer inspect(item);ref next=make Leaf {value<-prepare(flag);};item.child=next;return;}'
        'try{run(true);}catch(e:const NullPointerException){behave @print(31u64);};'
        'try{run(false);}catch(e:const NullPointerException){behave @print(41u64);};', 0, "7\n31\n19\n", ""),
    "deferred_product_rebinding_reaches_a_reference_slot_allocated_after_registration": (PRELUDE +
        'data Leaf {ref value:u64;}data Box {ref child:Leaf;}'
        'proc inspect(ref entry:const Box in O) where {const owner O}{behave @print(entry.child.value);return;}'
        'proc replace(ref entry:Box in O,ref replacement:u64 in O) where {owner O}{entry.child.value=replacement;return;}'
        'proc run()->void{val initial<-7u64;val replacement<-19u64;ref first=make Leaf {value=initial;};'
        'ref item=make Box {child=first;};defer inspect(item);defer replace(item,replacement);'
        'ref next=make Leaf {value=initial;};item.child=next;return;}run();', 0, "19\n", ""),
    "deferred_product_read_checks_an_ancestor_allocated_after_registration": (PRELUDE +
        'data Leaf {value:u64;}data Box {ptr child:Leaf;}'
        'proc inspect(ref entry:const Box in O) where {const owner O}{'
        'if(entry.child is null){return;}else{};behave @print(entry.child.value);return;}'
        'proc run()->void{ref first=make Leaf {value<-7u64;};ref item=make Box {child=first;};'
        'defer inspect(item);ptr next=make Leaf;item.child=next;return;}run();', 2, "", "initialization"),
    "deferred_product_read_can_use_the_ancestor_selected_by_a_registered_action": (PRELUDE +
        'data Leaf {value:u64;}data Box {ptr child:Leaf;}'
        'proc inspect(ref entry:const Box in O) where {const owner O}{'
        'if(entry.child is null){return;}else{};behave @print(entry.child.value);return;}'
        'proc run()->void{ptr first=make Leaf;ref second=make Leaf {value<-19u64;};ref item=make Box {child=null;};'
        'defer inspect(item);defer{item.child=second;};item.child=first;return;}run();', 0, "19\n", ""),
    "deferred_product_read_cannot_depend_on_a_later_registration_succeeding": (PRELUDE +
        'data Leaf {value:u64;}data Box {ptr child:Leaf;}'
        'proc inspect(ref entry:const Box in O) where {const owner O}{'
        'if(entry.child is null){return;}else{};behave @print(entry.child.value);return;}'
        'proc run()->void{ptr first=make Leaf;ref second=make Leaf {value<-19u64;};ref item=make Box {child=first;};'
        'defer inspect(item);defer{item.child=second;};return;}run();', 2, "", "initialization"),
    "deferred_product_rebinding_replaces_the_selected_slot": (PRELUDE +
        'data Leaf {ptr value:u64;}data Box {ref child:Leaf;}'
        'proc change(ref entry:Box in O,ref replacement:u64 in O) where {owner O}{entry.child.value=replacement;return;}'
        'proc inspect(ref entry:const Leaf in O) where {const owner O}{'
        'if(entry.value is null){return;}else{};behave @print(entry.value);return;}'
        'proc run()->void{val missing<-make local u64;val ready<-19u64;ref first=make Leaf {value=ready;};'
        'ref second=make Leaf {value=ready;};ref item=make Box {child=first;};defer inspect(second);'
        'defer change(item,ready);item.child=second;second.value=missing;return;}run();', 0, "19\n", ""),
    "deferred_product_rebinding_keeps_an_untouched_slot_readable": (PRELUDE +
        'data Leaf {ptr value:u64;}data Box {ref child:Leaf;}'
        'proc change(ref entry:Box in O,ptr replacement:u64 in O) where {owner O}{entry.child.value=replacement;return;}'
        'proc inspect(ref entry:const Leaf in O) where {const owner O}{'
        'if(entry.value is null){return;}else{};behave @print(entry.value);return;}'
        'proc run()->void{val missing<-make local u64;val ready<-19u64;ref first=make Leaf {value=ready;};'
        'ref second=make Leaf {value=ready;};ref item=make Box {child=second;};defer inspect(second);'
        'defer change(item,missing);item.child=first;return;}run();', 0, "19\n", ""),
    "deferred_product_rebinding_cannot_discharge_an_unselected_slot": (PRELUDE +
        'data Leaf {ptr value:u64;}data Box {ref child:Leaf;}'
        'proc change(ref entry:Box in O,ref replacement:u64 in O) where {owner O}{entry.child.value=replacement;return;}'
        'proc inspect(ref entry:const Leaf in O) where {const owner O}{'
        'if(entry.value is null){return;}else{};behave @print(entry.value);return;}'
        'proc run()->void{val missing<-make local u64;val ready<-19u64;ref first=make Leaf {value=ready;};'
        'ref second=make Leaf {value=ready;};ref item=make Box {child=second;};defer inspect(second);'
        'defer change(item,ready);item.child=first;second.value=missing;return;}run();', 2, "", "initialization"),
    "deferred_product_rebinding_uses_the_ancestor_selected_by_earlier_cleanup": (PRELUDE +
        'data Leaf {ptr value:u64;}data Box {ref child:Leaf;}'
        'proc change(ref entry:Box in O,ref replacement:u64 in O) where {owner O}{entry.child.value=replacement;return;}'
        'proc inspect(ref entry:const Leaf in O) where {const owner O}{'
        'if(entry.value is null){return;}else{};behave @print(entry.value);return;}'
        'proc run()->void{val missing<-make local u64;val ready<-19u64;ref first=make Leaf {value=ready;};'
        'ref second=make Leaf {value=ready;};ref item=make Box {child=first;};defer inspect(second);'
        'defer change(item,ready);defer{item.child=second;};second.value=missing;return;}run();', 0, "19\n", ""),
    "deferred_product_rebindings_compose_in_invocation_order": (PRELUDE +
        'data Leaf {ptr value:u64;}data Box {ref child:Leaf;}'
        'proc change(ref entry:Box in O,ptr replacement:u64 in O) where {owner O}{entry.child.value=replacement;return;}'
        'proc inspect(ref entry:const Leaf in O) where {const owner O}{'
        'if(entry.value is null){return;}else{};behave @print(entry.value);return;}'
        'proc run()->void{val missing<-make local u64;val ready<-19u64;ref first=make Leaf {value=ready;};'
        'ref second=make Leaf {value=ready;};ref item=make Box {child=first;};defer inspect(second);'
        'defer change(item,ready);defer change(item,missing);item.child=second;second.value=missing;return;}run();',
        0, "19\n", ""),
    "deferred_product_rebinding_has_no_destination_through_a_null_ancestor": (PRELUDE +
        'data Leaf {ptr value:u64;}data Box {ptr child:Leaf;}'
        'proc change(ref entry:Box in O,ref replacement:u64 in O) where {owner O}{'
        'if(entry.child is null){return;}else{};entry.child.value=replacement;return;}'
        'proc inspect(ref entry:const Leaf in O) where {const owner O}{'
        'if(entry.value is null){return;}else{};behave @print(entry.value);return;}'
        'proc run()->void{val missing<-make local u64;val ready<-19u64;ref second=make Leaf {value=ready;};'
        'ref item=make Box {child=second;};defer inspect(second);defer change(item,ready);'
        'item.child=null;second.value=missing;return;}run();', 2, "", "initialization"),
    "deferred_product_write_initializes_a_late_payload": (PRELUDE +
        'data Leaf {value:u64;}data Box {ptr child:Leaf;}'
        'proc fill(ref entry:Box in O) where {owner O}{if(entry.child is null){return;}else{};entry.child.value<-19u64;return;}'
        'proc inspect(ref entry:const Box in O) where {const owner O}{'
        'if(entry.child is null){return;}else{};behave @print(entry.child.value);return;}'
        'proc run()->void{ref first=make Leaf {value<-7u64;};ref item=make Box {child=first;};defer inspect(item);'
        'defer fill(item);ptr second=make Leaf;item.child=second;return;}run();', 0, "19\n", ""),
    "deferred_product_write_cannot_initialize_an_unselected_payload": (PRELUDE +
        'data Leaf {value:u64;}data Box {ptr child:Leaf;}'
        'proc fill(ref entry:Box in O) where {owner O}{if(entry.child is null){return;}else{};entry.child.value<-19u64;return;}'
        'proc inspect(ref entry:const Box in O) where {const owner O}{'
        'if(entry.child is null){return;}else{};behave @print(entry.child.value);return;}'
        'proc run(flag:bool)->void{ref first=make Leaf {value<-7u64;};ref item=make Box {child=first;};'
        'ref writer=make Box {child=first;};defer inspect(item);defer fill(writer);ptr second=make Leaf;'
        'item.child=second;if(flag){writer.child=second;{}}else{};return;}run(false);', 2, "", "initialization"),
    "deferred_product_binding_can_initialize_a_later_slot": (PRELUDE +
        'data Leaf {ptr value:u64;}data Box {ptr child:Leaf;}'
        'proc fill(ref entry:Box in O,ptr replacement:u64 in O) where {owner O}{'
        'if(entry.child is null){return;}else{};entry.child.value=replacement;return;}'
        'proc inspect(ref entry:const Box in O) where {const owner O}{if(entry.child is null){return;}else{};'
        'if(entry.child.value is null){return;}else{};behave @print(entry.child.value);return;}'
        'proc run()->void{val ready<-19u64;ref first=make Leaf {value=ready;};ref item=make Box {child=first;};'
        'defer inspect(item);defer fill(item,ready);ptr second=make Leaf;item.child=second;return;}run();', 0, "19\n", ""),
    "deferred_product_binding_does_not_initialize_the_referent": (PRELUDE +
        'data Leaf {ptr value:u64;}data Box {ptr child:Leaf;}'
        'proc fill(ref entry:Box in O,ptr replacement:u64 in O) where {owner O}{'
        'if(entry.child is null){return;}else{};entry.child.value=replacement;return;}'
        'proc inspect(ref entry:const Box in O) where {const owner O}{if(entry.child is null){return;}else{};'
        'if(entry.child.value is null){return;}else{};behave @print(entry.child.value);return;}'
        'proc run()->void{val ready<-19u64;ref first=make Leaf {value=ready;};ref item=make Box {child=first;};'
        'val missing<-make local u64;defer inspect(item);defer fill(item,missing);ptr second=make Leaf;'
        'item.child=second;return;}run();', 2, "", "initialization"),
    "deferred_product_read_cannot_depend_on_a_fallible_write_succeeding": (PRELUDE +
        'data Leaf {value:u64;}data Box {ptr child:Leaf;}proc noop()->void{return;}'
        'proc fill(ref entry:Box in O) where {owner O}{if(entry.child is null){return;}else{};'
        'defer noop();entry.child.value<-19u64;return;}'
        'proc inspect(ref entry:const Box in O) where {const owner O}{'
        'if(entry.child is null){return;}else{};behave @print(entry.child.value);return;}'
        'proc run()->void{ref first=make Leaf {value<-7u64;};ref item=make Box {child=first;};defer inspect(item);'
        'defer fill(item);ptr second=make Leaf;item.child=second;return;}run();', 2, "", "initialization"),
    "different_saved_receivers_can_initialize_the_same_payload": (PRELUDE +
        'data Leaf {value:u64;}data Box {ptr child:Leaf;}'
        'proc fill(ref entry:Box in O) where {owner O}{if(entry.child is null){return;}else{};entry.child.value<-19u64;return;}'
        'proc inspect(ref entry:const Box in O) where {const owner O}{'
        'if(entry.child is null){return;}else{};behave @print(entry.child.value);return;}'
        'proc run(flag:bool)->void{ref first=make Leaf {value<-7u64;};ref item=make Box {child=first;};'
        'ref alternate=make Box {child=first;};ptr chosen=if(flag){item}else{alternate};defer inspect(item);defer fill(chosen);'
        'ptr second=make Leaf;item.child=second;alternate.child=second;return;}run(true);run(false);', 0, "19\n19\n", ""),
    "convergent_rebinding_uses_the_saved_receiver_choices": (PRELUDE +
        'data Leaf {ptr value:u64;}data Box {ref child:Leaf;}'
        'proc change(ref entry:Box in O,ref replacement:u64 in O) where {owner O}{entry.child.value=replacement;return;}'
        'proc inspect(ref entry:const Leaf in O) where {const owner O}{'
        'if(entry.value is null){return;}else{};behave @print(entry.value);return;}'
        'proc run(flag:bool)->void{val missing<-make local u64;val ready<-19u64;ref first=make Leaf {value=ready;};'
        'ref second=make Leaf {value=ready;};ref left=make Box {child=first;};ref right=make Box {child=first;};'
        'ptr chosen=if(flag){left}else{right};ref later=make Box {child=first;};defer inspect(second);defer change(chosen,ready);'
        'left.child=second;right.child=second;chosen=later;second.value=missing;return;}run(true);run(false);', 0, "19\n19\n", ""),
    "a_nullable_ancestor_in_one_saved_receiver_can_skip_initialization": (PRELUDE +
        'data Leaf {value:u64;}data Box {ptr child:Leaf;}'
        'proc fill(ref entry:Box in O) where {owner O}{if(entry.child is null){return;}else{};entry.child.value<-19u64;return;}'
        'proc inspect(ref entry:const Box in O) where {const owner O}{'
        'if(entry.child is null){return;}else{};behave @print(entry.child.value);return;}'
        'proc run(flag:bool)->void{ref first=make Leaf {value<-7u64;};ref item=make Box {child=first;};'
        'ref alternate=make Box {child=first;};ptr chosen=if(flag){item}else{alternate};defer inspect(item);defer fill(chosen);'
        'ptr second=make Leaf;item.child=second;alternate.child=null;return;}run(false);', 2, "", "initialization"),
}
runner.CASE_ARGUMENTS = {"shadow_error": ["-Werror=shadow"], "unrelated_warning_error": ["-Werror=missing-return"]}
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == "__main__":
    sys.exit(runner.main())
