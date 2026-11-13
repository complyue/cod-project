%Completion = type { i1, i64 }
@binding = thread_local global i64 0
@acquired = global i64 0
@released = global i64 0
@event_format = private constant [15 x i8] c"E %ld %ld %ld\0A\00"
@result_format = private constant [28 x i8] c"RESULT %ld %ld %ld %ld %ld\0A\00"
declare i32 @printf(ptr, ...)
declare void @exit(i32) noreturn
declare i64 @strtoll(ptr, ptr, i32)
define void @event(i64 %tag, i64 %a, i64 %b) {
  %p = call i32 (ptr, ...) @printf(ptr @event_format, i64 %tag, i64 %a, i64 %b)
  ret void
}
define %Completion @checkout(i64 %arg_n, i64 %arg_source, i64 %arg_fail_at, i64 %arg_cleanup_at, i64 %arg_mode) {
entry:
  %thrown = alloca i64
  store i64 0, ptr %thrown
  %pending = alloca i64
  store i64 0, ptr %pending
  %has_return = alloca i64
  store i64 0, ptr %has_return
  %saved_return = alloca i64
  store i64 0, ptr %saved_return
  %saved_view = alloca i64
  store i64 0, ptr %saved_view
  %local_available = alloca i64
  store i64 0, ptr %local_available
  %local_captured = alloca i64
  store i64 0, ptr %local_captured
  %local_captured_view = alloca i64
  store i64 0, ptr %local_captured_view
  %local_cleanup_at = alloca i64
  store i64 0, ptr %local_cleanup_at
  %local_error = alloca i64
  store i64 0, ptr %local_error
  %local_fail_at = alloca i64
  store i64 0, ptr %local_fail_at
  %local_lease = alloca i64
  store i64 0, ptr %local_lease
  %local_local_view = alloca i64
  store i64 0, ptr %local_local_view
  %local_mode = alloca i64
  store i64 0, ptr %local_mode
  %local_n = alloca i64
  store i64 0, ptr %local_n
  %local_rest = alloca i64
  store i64 0, ptr %local_rest
  %local_restored = alloca i64
  store i64 0, ptr %local_restored
  %local_score = alloca i64
  store i64 0, ptr %local_score
  %local_source = alloca i64
  store i64 0, ptr %local_source
  %registered_release = alloca i64
  store i64 0, ptr %registered_release
  %registered_recover = alloca i64
  store i64 0, ptr %registered_recover
  %registered_cleanup_failure = alloca i64
  store i64 0, ptr %registered_cleanup_failure
  %live_lease = alloca i64
  store i64 0, ptr %live_lease
  %v1 = load i64, ptr @binding
  store i64 %v1, ptr %saved_view
  store i64 %arg_n, ptr %local_n
  store i64 %arg_source, ptr %local_source
  store i64 %arg_fail_at, ptr %local_fail_at
  store i64 %arg_cleanup_at, ptr %local_cleanup_at
  store i64 %arg_mode, ptr %local_mode
  %v2 = load i64, ptr %local_n
  %v3 = icmp eq i64 %v2, 0
  %v4 = zext i1 %v3 to i64
  %v5 = icmp ne i64 %v4, 0
  br i1 %v5, label %yes6, label %no7
yes6:
  store i64 0, ptr %thrown
  store i64 0, ptr %pending
  store i64 1, ptr %has_return
  store i64 0, ptr %saved_return
  br label %cleanup_cleanup_failure
no7:
  br label %join8
join8:
  %v9 = load i64, ptr @acquired
  %v10 = add i64 %v9, 1
  store i64 %v10, ptr @acquired
  store i64 %v10, ptr %local_lease
  store i64 1, ptr %live_lease
  store i64 1, ptr %registered_release
  store i64 1, ptr %registered_recover
  store i64 1, ptr %registered_cleanup_failure
  %v11 = load i64, ptr @binding
  store i64 %v11, ptr %local_captured_view
  %v12 = load i64, ptr %local_n
  %v13 = load i64, ptr %local_source
  %v14 = add i64 %v12, %v13
  %v15 = srem i64 %v14, 3
  %v16 = icmp eq i64 %v15, 0
  %v17 = zext i1 %v16 to i64
  %v18 = load i64, ptr %local_n
  %v19 = load i64, ptr %local_source
  %v20 = add i64 %v18, %v19
  %v21 = srem i64 %v20, 3
  %v22 = icmp eq i64 %v21, 1
  %v23 = zext i1 %v22 to i64
  %v24 = icmp ne i64 %v23, 0
  %v25 = select i1 %v24, i64 1, i64 2
  %v26 = icmp ne i64 %v17, 0
  %v27 = select i1 %v26, i64 0, i64 %v25
  store i64 %v27, ptr %local_local_view
  %v28 = load i64, ptr %local_local_view
  store i64 %v28, ptr @binding
  %v29 = load i64, ptr %local_n
  %v30 = load i64, ptr %local_local_view
  call void @event(i64 10, i64 %v29, i64 %v30)
  %v31 = load i64, ptr %local_n
  %v32 = load i64, ptr %local_fail_at
  %v33 = icmp eq i64 %v31, %v32
  %v34 = zext i1 %v33 to i64
  %v35 = icmp ne i64 %v34, 0
  br i1 %v35, label %yes36, label %no37
yes36:
  store i64 1, ptr %thrown
  store i64 902, ptr %pending
  br label %cleanup_cleanup_failure
no37:
  br label %join38
join38:
  %v39 = load i64, ptr %local_n
  %v40 = load i64, ptr %local_captured_view
  %v41 = icmp eq i64 %v40, 1
  %v42 = select i1 %v41, ptr @stock_a, ptr @stock_b
  %v43 = icmp eq i64 %v40, 0
  %v44 = select i1 %v43, ptr @stock_base, ptr %v42
  %v45 = call %Completion %v44(i64 %v39)
  %v46 = extractvalue %Completion %v45, 0
  %v47 = extractvalue %Completion %v45, 1
  br i1 %v46, label %failed48, label %passed49
failed48:
  store i64 1, ptr %thrown
  store i64 %v47, ptr %pending
  br label %cleanup_cleanup_failure
passed49:
  store i64 %v47, ptr %local_captured
  %v50 = load i64, ptr %local_n
  %v51 = load i64, ptr %local_source
  %v52 = call %Completion @dependency_helper(i64 %v50, i64 %v51)
  %v53 = extractvalue %Completion %v52, 0
  %v54 = extractvalue %Completion %v52, 1
  br i1 %v53, label %failed55, label %passed56
failed55:
  store i64 1, ptr %thrown
  store i64 %v54, ptr %pending
  br label %cleanup_cleanup_failure
passed56:
  store i64 %v54, ptr %local_available
  %v57 = load i64, ptr %local_n
  %v58 = load i64, ptr @binding
  %v59 = icmp eq i64 %v58, 1
  %v60 = select i1 %v59, ptr @stock_a, ptr @stock_b
  %v61 = icmp eq i64 %v58, 0
  %v62 = select i1 %v61, ptr @stock_base, ptr %v60
  %v63 = call %Completion %v62(i64 %v57)
  %v64 = extractvalue %Completion %v63, 0
  %v65 = extractvalue %Completion %v63, 1
  br i1 %v64, label %failed66, label %passed67
failed66:
  store i64 1, ptr %thrown
  store i64 %v65, ptr %pending
  br label %cleanup_cleanup_failure
passed67:
  store i64 %v65, ptr %local_restored
  %v68 = load i64, ptr %local_captured
  %v69 = load i64, ptr %local_available
  call void @event(i64 21, i64 %v68, i64 %v69)
  %v70 = load i64, ptr %local_restored
  %v71 = load i64, ptr @binding
  call void @event(i64 22, i64 %v70, i64 %v71)
  store i64 1, ptr %local_score
  %v72 = load i64, ptr %local_available
  %v73 = icmp ne i64 %v72, 0
  br i1 %v73, label %yes74, label %no75
yes74:
  %v77 = load i64, ptr %local_n
  store i64 %v77, ptr %local_score
  br label %join76
no75:
  br label %join76
join76:
  %v78 = load i64, ptr %local_captured
  %v79 = icmp ne i64 %v78, 0
  br i1 %v79, label %yes80, label %no81
yes80:
  %v83 = load i64, ptr %local_score
  %v84 = add i64 %v83, 2
  store i64 %v84, ptr %local_score
  br label %join82
no81:
  br label %join82
join82:
  %v85 = load i64, ptr %local_n
  %v86 = sub i64 %v85, 1
  %v87 = load i64, ptr %local_source
  %v88 = load i64, ptr %local_fail_at
  %v89 = load i64, ptr %local_cleanup_at
  %v90 = load i64, ptr %local_mode
  %v91 = call %Completion @checkout(i64 %v86, i64 %v87, i64 %v88, i64 %v89, i64 %v90)
  %v92 = extractvalue %Completion %v91, 0
  %v93 = extractvalue %Completion %v91, 1
  br i1 %v92, label %failed94, label %passed95
failed94:
  store i64 1, ptr %thrown
  store i64 %v93, ptr %pending
  br label %cleanup_cleanup_failure
passed95:
  store i64 %v93, ptr %local_rest
  %v96 = load i64, ptr %local_score
  %v97 = load i64, ptr %local_rest
  %v98 = add i64 %v96, %v97
  store i64 0, ptr %thrown
  store i64 %v98, ptr %pending
  store i64 1, ptr %has_return
  store i64 %v98, ptr %saved_return
  br label %cleanup_cleanup_failure
cleanup_cleanup_failure:
  %v99 = load i64, ptr %registered_cleanup_failure
  %v100 = icmp ne i64 %v99, 0
  br i1 %v100, label %cleanup_cleanup_failure_dispatch, label %cleanup_recover
cleanup_cleanup_failure_dispatch:
  %v101 = load i64, ptr %thrown
  %v102 = icmp ne i64 %v101, 0
  br i1 %v102, label %cleanup_cleanup_failure_finally, label %cleanup_cleanup_failure_body
cleanup_cleanup_failure_body:
  %v103 = load i64, ptr %local_n
  %v104 = load i64, ptr %local_cleanup_at
  %v105 = icmp eq i64 %v103, %v104
  %v106 = zext i1 %v105 to i64
  %v107 = icmp ne i64 %v106, 0
  br i1 %v107, label %yes108, label %no109
yes108:
  store i64 1, ptr %thrown
  store i64 901, ptr %pending
  br label %cleanup_cleanup_failure_finally
no109:
  br label %join110
join110:
  br label %cleanup_cleanup_failure_finally
cleanup_cleanup_failure_finally:
  %v111 = load i64, ptr %local_n
  call void @event(i64 30, i64 %v111, i64 0)
  br label %cleanup_recover
cleanup_recover:
  %v112 = load i64, ptr %registered_recover
  %v113 = icmp ne i64 %v112, 0
  br i1 %v113, label %cleanup_recover_dispatch, label %cleanup_release
cleanup_recover_dispatch:
  %v114 = load i64, ptr %thrown
  %v115 = icmp ne i64 %v114, 0
  br i1 %v115, label %cleanup_recover_catch, label %cleanup_recover_body
cleanup_recover_body:
  br label %cleanup_recover_finally
cleanup_recover_catch:
  %v116 = load i64, ptr %pending
  store i64 %v116, ptr %local_error
  %v117 = load i64, ptr %local_n
  %v118 = load i64, ptr %local_error
  call void @event(i64 40, i64 %v117, i64 %v118)
  %v119 = load i64, ptr %local_mode
  %v120 = icmp eq i64 %v119, 0
  %v121 = zext i1 %v120 to i64
  %v122 = icmp ne i64 %v121, 0
  br i1 %v122, label %yes123, label %no124
yes123:
  %v126 = load i64, ptr %local_error
  store i64 1, ptr %thrown
  store i64 %v126, ptr %pending
  br label %cleanup_recover_finally
no124:
  br label %join125
join125:
  %v127 = load i64, ptr %local_mode
  %v128 = icmp eq i64 %v127, 2
  %v129 = zext i1 %v128 to i64
  %v130 = icmp ne i64 %v129, 0
  br i1 %v130, label %yes131, label %no132
yes131:
  store i64 0, ptr %thrown
  store i64 777, ptr %pending
  store i64 1, ptr %has_return
  store i64 777, ptr %saved_return
  br label %cleanup_recover_finally
no132:
  br label %join133
join133:
  store i64 0, ptr %thrown
  %v134 = load i64, ptr %saved_return
  store i64 %v134, ptr %pending
  br label %cleanup_recover_finally
cleanup_recover_finally:
  %v135 = load i64, ptr %local_n
  %v136 = load i64, ptr %local_mode
  call void @event(i64 41, i64 %v135, i64 %v136)
  %v137 = load i64, ptr %local_mode
  %v138 = icmp eq i64 %v137, 3
  %v139 = zext i1 %v138 to i64
  %v140 = icmp ne i64 %v139, 0
  br i1 %v140, label %yes141, label %no142
yes141:
  store i64 0, ptr %thrown
  store i64 888, ptr %pending
  store i64 1, ptr %has_return
  store i64 888, ptr %saved_return
  br label %cleanup_release
no142:
  br label %join143
join143:
  br label %cleanup_release
cleanup_release:
  %v144 = load i64, ptr %registered_release
  %v145 = icmp ne i64 %v144, 0
  br i1 %v145, label %cleanup_release_dispatch, label %exit
cleanup_release_dispatch:
  %v146 = load i64, ptr %thrown
  %v147 = icmp ne i64 %v146, 0
  br i1 %v147, label %cleanup_release_finally, label %cleanup_release_body
cleanup_release_body:
  br label %cleanup_release_finally
cleanup_release_finally:
  %v148 = load i64, ptr %local_n
  %v149 = load i64, ptr %local_lease
  call void @event(i64 50, i64 %v148, i64 %v149)
  %v150 = load i64, ptr %local_lease
  %v151 = load i64, ptr %live_lease
  %v152 = load i64, ptr %local_lease
  %v153 = icmp eq i64 %v150, %v152
  %v154 = icmp ne i64 %v151, 0
  %v155 = and i1 %v153, %v154
  %v156 = or i1 false, %v155
  %v157 = select i1 %v155, i64 0, i64 %v151
  store i64 %v157, ptr %live_lease
  br i1 %v156, label %release_ok158, label %release_bad159
release_bad159:
  call void @exit(i32 97)
  unreachable
release_ok158:
  %v160 = load i64, ptr @released
  %v161 = add i64 %v160, 1
  store i64 %v161, ptr @released
  br label %exit
exit:
  %v162 = load i64, ptr %live_lease
  %v163 = icmp ne i64 %v162, 0
  br i1 %v163, label %leaked165, label %released164
leaked165:
  call void @exit(i32 97)
  unreachable
released164:
  %v166 = load i64, ptr %saved_view
  store i64 %v166, ptr @binding
  %v167 = load i64, ptr %thrown
  %v168 = icmp ne i64 %v167, 0
  %v169 = load i64, ptr %has_return
  %v170 = icmp ne i64 %v169, 0
  %v171 = xor i1 %v170, true
  %v172 = or i1 %v168, %v171
  %v173 = load i64, ptr %pending
  %v174 = select i1 %v170, i64 %v173, i64 903
  %v175 = load i64, ptr %pending
  %v176 = select i1 %v168, i64 %v175, i64 %v174
  %v177 = insertvalue %Completion poison, i1 %v172, 0
  %v178 = insertvalue %Completion %v177, i64 %v176, 1
  ret %Completion %v178
}
define %Completion @dependency_helper(i64 %arg_n, i64 %arg_source) {
entry:
  %thrown = alloca i64
  store i64 0, ptr %thrown
  %pending = alloca i64
  store i64 0, ptr %pending
  %has_return = alloca i64
  store i64 0, ptr %has_return
  %saved_return = alloca i64
  store i64 0, ptr %saved_return
  %saved_view = alloca i64
  store i64 0, ptr %saved_view
  %local_available = alloca i64
  store i64 0, ptr %local_available
  %local_error = alloca i64
  store i64 0, ptr %local_error
  %local_local_view = alloca i64
  store i64 0, ptr %local_local_view
  %local_n = alloca i64
  store i64 0, ptr %local_n
  %local_source = alloca i64
  store i64 0, ptr %local_source
  %v179 = load i64, ptr @binding
  store i64 %v179, ptr %saved_view
  store i64 %arg_n, ptr %local_n
  store i64 %arg_source, ptr %local_source
  %v180 = load i64, ptr %local_n
  %v181 = load i64, ptr %local_source
  %v182 = add i64 %v180, %v181
  %v183 = srem i64 %v182, 2
  %v184 = icmp eq i64 %v183, 0
  %v185 = zext i1 %v184 to i64
  %v186 = icmp ne i64 %v185, 0
  %v187 = select i1 %v186, i64 1, i64 2
  store i64 %v187, ptr %local_local_view
  %v188 = load i64, ptr %local_local_view
  store i64 %v188, ptr @binding
  %v189 = load i64, ptr %local_n
  %v190 = load i64, ptr %local_local_view
  call void @event(i64 20, i64 %v189, i64 %v190)
  %v191 = load i64, ptr %local_n
  %v192 = load i64, ptr @binding
  %v193 = icmp eq i64 %v192, 1
  %v194 = select i1 %v193, ptr @stock_a, ptr @stock_b
  %v195 = icmp eq i64 %v192, 0
  %v196 = select i1 %v195, ptr @stock_base, ptr %v194
  %v197 = call %Completion %v196(i64 %v191)
  %v198 = extractvalue %Completion %v197, 0
  %v199 = extractvalue %Completion %v197, 1
  br i1 %v198, label %failed200, label %passed201
failed200:
  store i64 1, ptr %thrown
  store i64 %v199, ptr %pending
  br label %exit
passed201:
  store i64 %v199, ptr %local_available
  %v202 = load i64, ptr %local_available
  store i64 0, ptr %thrown
  store i64 %v202, ptr %pending
  store i64 1, ptr %has_return
  store i64 %v202, ptr %saved_return
  br label %exit
exit:
  %v203 = load i64, ptr %saved_view
  store i64 %v203, ptr @binding
  %v204 = load i64, ptr %thrown
  %v205 = icmp ne i64 %v204, 0
  %v206 = load i64, ptr %has_return
  %v207 = icmp ne i64 %v206, 0
  %v208 = xor i1 %v207, true
  %v209 = or i1 %v205, %v208
  %v210 = load i64, ptr %pending
  %v211 = select i1 %v207, i64 %v210, i64 903
  %v212 = load i64, ptr %pending
  %v213 = select i1 %v205, i64 %v212, i64 %v211
  %v214 = insertvalue %Completion poison, i1 %v209, 0
  %v215 = insertvalue %Completion %v214, i64 %v213, 1
  ret %Completion %v215
}
define %Completion @stock_a(i64 %arg_n) {
entry:
  %thrown = alloca i64
  store i64 0, ptr %thrown
  %pending = alloca i64
  store i64 0, ptr %pending
  %has_return = alloca i64
  store i64 0, ptr %has_return
  %saved_return = alloca i64
  store i64 0, ptr %saved_return
  %saved_view = alloca i64
  store i64 0, ptr %saved_view
  %local_error = alloca i64
  store i64 0, ptr %local_error
  %local_n = alloca i64
  store i64 0, ptr %local_n
  %v216 = load i64, ptr @binding
  store i64 %v216, ptr %saved_view
  store i64 %arg_n, ptr %local_n
  %v217 = load i64, ptr %local_n
  %v218 = srem i64 %v217, 3
  %v219 = icmp eq i64 %v218, 0
  %v220 = zext i1 %v219 to i64
  store i64 0, ptr %thrown
  store i64 %v220, ptr %pending
  store i64 1, ptr %has_return
  store i64 %v220, ptr %saved_return
  br label %exit
exit:
  %v221 = load i64, ptr %saved_view
  store i64 %v221, ptr @binding
  %v222 = load i64, ptr %thrown
  %v223 = icmp ne i64 %v222, 0
  %v224 = load i64, ptr %has_return
  %v225 = icmp ne i64 %v224, 0
  %v226 = xor i1 %v225, true
  %v227 = or i1 %v223, %v226
  %v228 = load i64, ptr %pending
  %v229 = select i1 %v225, i64 %v228, i64 903
  %v230 = load i64, ptr %pending
  %v231 = select i1 %v223, i64 %v230, i64 %v229
  %v232 = insertvalue %Completion poison, i1 %v227, 0
  %v233 = insertvalue %Completion %v232, i64 %v231, 1
  ret %Completion %v233
}
define %Completion @stock_b(i64 %arg_n) {
entry:
  %thrown = alloca i64
  store i64 0, ptr %thrown
  %pending = alloca i64
  store i64 0, ptr %pending
  %has_return = alloca i64
  store i64 0, ptr %has_return
  %saved_return = alloca i64
  store i64 0, ptr %saved_return
  %saved_view = alloca i64
  store i64 0, ptr %saved_view
  %local_error = alloca i64
  store i64 0, ptr %local_error
  %local_n = alloca i64
  store i64 0, ptr %local_n
  %v234 = load i64, ptr @binding
  store i64 %v234, ptr %saved_view
  store i64 %arg_n, ptr %local_n
  %v235 = load i64, ptr %local_n
  %v236 = srem i64 %v235, 5
  %v237 = icmp eq i64 %v236, 0
  %v238 = zext i1 %v237 to i64
  store i64 0, ptr %thrown
  store i64 %v238, ptr %pending
  store i64 1, ptr %has_return
  store i64 %v238, ptr %saved_return
  br label %exit
exit:
  %v239 = load i64, ptr %saved_view
  store i64 %v239, ptr @binding
  %v240 = load i64, ptr %thrown
  %v241 = icmp ne i64 %v240, 0
  %v242 = load i64, ptr %has_return
  %v243 = icmp ne i64 %v242, 0
  %v244 = xor i1 %v243, true
  %v245 = or i1 %v241, %v244
  %v246 = load i64, ptr %pending
  %v247 = select i1 %v243, i64 %v246, i64 903
  %v248 = load i64, ptr %pending
  %v249 = select i1 %v241, i64 %v248, i64 %v247
  %v250 = insertvalue %Completion poison, i1 %v245, 0
  %v251 = insertvalue %Completion %v250, i64 %v249, 1
  ret %Completion %v251
}
define %Completion @stock_base(i64 %arg_n) {
entry:
  %thrown = alloca i64
  store i64 0, ptr %thrown
  %pending = alloca i64
  store i64 0, ptr %pending
  %has_return = alloca i64
  store i64 0, ptr %has_return
  %saved_return = alloca i64
  store i64 0, ptr %saved_return
  %saved_view = alloca i64
  store i64 0, ptr %saved_view
  %local_error = alloca i64
  store i64 0, ptr %local_error
  %local_n = alloca i64
  store i64 0, ptr %local_n
  %v252 = load i64, ptr @binding
  store i64 %v252, ptr %saved_view
  store i64 %arg_n, ptr %local_n
  %v253 = load i64, ptr %local_n
  %v254 = srem i64 %v253, 2
  %v255 = icmp eq i64 %v254, 0
  %v256 = zext i1 %v255 to i64
  store i64 0, ptr %thrown
  store i64 %v256, ptr %pending
  store i64 1, ptr %has_return
  store i64 %v256, ptr %saved_return
  br label %exit
exit:
  %v257 = load i64, ptr %saved_view
  store i64 %v257, ptr @binding
  %v258 = load i64, ptr %thrown
  %v259 = icmp ne i64 %v258, 0
  %v260 = load i64, ptr %has_return
  %v261 = icmp ne i64 %v260, 0
  %v262 = xor i1 %v261, true
  %v263 = or i1 %v259, %v262
  %v264 = load i64, ptr %pending
  %v265 = select i1 %v261, i64 %v264, i64 903
  %v266 = load i64, ptr %pending
  %v267 = select i1 %v259, i64 %v266, i64 %v265
  %v268 = insertvalue %Completion poison, i1 %v263, 0
  %v269 = insertvalue %Completion %v268, i64 %v267, 1
  ret %Completion %v269
}
define i32 @main(i32 %argc, ptr %argv) {
entry:
  %v270 = icmp eq i32 %argc, 6
  br i1 %v270, label %run, label %bad
bad:
  ret i32 2
run:
  %v271 = getelementptr ptr, ptr %argv, i64 1
  %v272 = load ptr, ptr %v271
  %v273 = call i64 @strtoll(ptr %v272, ptr null, i32 10)
  %v274 = getelementptr ptr, ptr %argv, i64 2
  %v275 = load ptr, ptr %v274
  %v276 = call i64 @strtoll(ptr %v275, ptr null, i32 10)
  %v277 = getelementptr ptr, ptr %argv, i64 3
  %v278 = load ptr, ptr %v277
  %v279 = call i64 @strtoll(ptr %v278, ptr null, i32 10)
  %v280 = getelementptr ptr, ptr %argv, i64 4
  %v281 = load ptr, ptr %v280
  %v282 = call i64 @strtoll(ptr %v281, ptr null, i32 10)
  %v283 = getelementptr ptr, ptr %argv, i64 5
  %v284 = load ptr, ptr %v283
  %v285 = call i64 @strtoll(ptr %v284, ptr null, i32 10)
  store i64 0, ptr @binding
  %v286 = call %Completion @checkout(i64 %v273, i64 %v276, i64 %v279, i64 %v282, i64 %v285)
  %v287 = extractvalue %Completion %v286, 0
  %v288 = zext i1 %v287 to i64
  %v289 = extractvalue %Completion %v286, 1
  %v290 = load i64, ptr @binding
  %v291 = load i64, ptr @acquired
  %v292 = load i64, ptr @released
  call i32 (ptr, ...) @printf(ptr @result_format, i64 %v288, i64 %v289, i64 %v290, i64 %v291, i64 %v292)
  ret i32 0
}
