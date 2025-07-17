# Execute Command Tool — Quick Rules

`cwd` decides **which VSCode terminal tab is used** and **what directory the command runs in**. Follow these condensed rules to keep terminal behaviour predictable.

## 1. Core Concepts  
• One unique `cwd` → one persistent terminal tab (env vars & history saved)  
• Same `cwd` → same tab   • Different `cwd`s → isolated tabs  
• Before each command the tab is reset to its `cwd`, so prior `cd` changes are ignored

## 2. Golden Rules  
1. **Always set `cwd`** when directory matters—never rely on “current” path.  
2. **Group related work** under the same `cwd` to reuse env & history.  
3. **Avoid `cd` between commands**. Use another `cwd` instead.  
4. **Use `cd` only inside one compound command** when truly needed (e.g. `cd /tmp && …`).  
5. Environment variables live only in their tab; don’t expect them across `cwd`s.  
6. Long-running tasks stay in their tab—remember to stop/clean them.  
7. Don’t create lots of random `cwd`s; stick to logical project roots.  
8. Never mix strategies: if you changed dir with `cd`, the next call with the same `cwd` will snap back—plan for it.  
9. Don’t assume a fresh shell; check/reset state if required.  
10. Treat `cwd` as both *ID* and *directory lock* for a tab.

## 3. Best-Practice Templates
```xml
<!-- Build frontend → same tab reused for all frontend commands -->
<execute_command>
<command>npm install</command>
<cwd>/Users/cyue/Documents/project/frontend</cwd>
</execute_command>

<execute_command>
<command>npm run build</command>
<cwd>/Users/cyue/Documents/project/frontend</cwd>
</execute_command>

<!-- Complex single command needing temp dir -->
<execute_command>
<command>cd /tmp && git clone repo.git && cd repo && npm install</command>
<cwd>/Users/cyue/Documents/project</cwd>
</execute_command>
```

## 4. Anti-Patterns to Avoid  
✘ Using `cd` to “switch context” across multiple calls   ✘ Relying on implicit working dir  
✘ Spawning excessive tabs by sprinkling unique `cwd`s   ✘ Expecting env vars to cross tabs  
