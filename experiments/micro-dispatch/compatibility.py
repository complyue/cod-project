"""Optional planning observation; no DBMR format, loader or migration changes."""
import hashlib
from dataclasses import replace
from model import Type, Universe


def checksum(universe, root):
    return hashlib.sha256(universe.canonical(root).encode()).hexdigest()


def observations():
    graph=[Type('Node',size=24,align=8,fields=(('value',0,'i64',()),('next',8,'reference',('Node',)),('shared',16,'reference',('Leaf',)))),
           Type('Leaf',size=8,align=8,fields=(('value',0,'i64',()),))]
    original=Universe(graph)
    reordered=Universe(list(reversed(graph)))
    extra=Universe([Type('AUnrelated',size=8,align=8),*graph])
    altered=Universe([replace(graph[0],align=16),graph[1]])
    assert checksum(original,'Node') == checksum(reordered,'Node') == checksum(extra,'Node')
    assert original.type_ids['Leaf'] == extra.type_ids['Leaf']
    naive_old={name:i for i,name in enumerate(sorted(original.types))}
    naive_new={name:i for i,name in enumerate(sorted(extra.types))}
    assert naive_old['Leaf'] != naive_new['Leaf']
    bad=Universe(graph,codes={'Node':original.type_ids['Leaf'],'Leaf':original.type_ids['Node']})
    assert checksum(original,'Node') == checksum(bad,'Node')
    assert checksum(original,'Node') != checksum(altered,'Node')
    return {'role':'illustrative planning only; checksum not used by runtime dispatch or DBMR loader',
            'root':'Node','checksum':checksum(original,'Node'),
            'recursive_shared_named_graph':'finite and declaration-order independent',
            'unrelated_type_added':'same root checksum and stable IDs; naked build ordinals would change interpretation',
            'compatibility':'equal checksum also possible with a deliberately wrong ID binding; identity/schema/map/ABI/bounds remain separate',
            'layout_changed_checksum':checksum(altered,'Node')}
