
commit -m  "cpp-"

On branch dev
Your branch is up to date with 'origin/dev'.

Changes to be committed:
  (use "git restore --staged <file>..." to unstage)
	renamed:    main_wrapper.c -> print_test.c

Changes not staged for commit:
  (use "git add/rm <file>..." to update what will be committed)
  (use "git restore <file>..." to discard changes in working directory)
	modified:   main.cpp
	modified:   print_test
	deleted:    print_test.cpp
	modified:   test/basic_test.sh
	modified:   test/case2/1.c
	modified:   test/case2/10.c
	modified:   test/case2/11.c
	modified:   test/case2/2.c
	modified:   test/case2/3.c
	modified:   test/case2/4.c
	modified:   test/case2/5.c
	modified:   test/case2/6.c
	modified:   test/case2/7.c
	modified:   test/case2/8.c
	modified:   test/case2/9.c
	modified:   test/case3_vsl_alloc_cases/01_matrix_pressure_4x4.c
	modified:   test/case3_vsl_alloc_cases/02_snapshot_rotation_pressure.c
	modified:   test/case3_vsl_alloc_cases/03_cond_body_aliasing.c
	modified:   test/case3_vsl_alloc_cases/04_zero_trip_then_siblings.c
	modified:   test/case3_vsl_alloc_cases/05_four_level_live_ranges.c
	modified:   test/case3_vsl_alloc_cases/06_deep_mixed_access.c
	modified:   test/case3_vsl_alloc_cases/07_nested_cond_body_aliasing.c
	modified:   test/case3_vsl_alloc_cases/08_four_level_counter_alias_pressure.c
	modified:   test/case3_vsl_alloc_cases/09_condition_limit_body_write.c
	modified:   test/case3_vsl_alloc_cases/10_lvalue_and_step_alias.c
	modified:   test/case3_vsl_alloc_cases/11_zero_trip_and_sibling_reentry.c
	modified:   test/case3_vsl_alloc_cases/12_condition_lhs_rhs_mutated.c
	modified:   test/case3_vsl_alloc_cases/13_cond_spill_covers_dirty_vr.c
	modified:   test/case3_vsl_alloc_cases/14_body_tail_must_spill.c
	modified:   test/case3_vsl_alloc_cases/15_deep_if_dirty_inside_nested_loops.c
	modified:   test/case3_vsl_alloc_cases/16_many_live_values_branch_rejoin.c
	modified:   test/case3_vsl_alloc_cases/17_loop_inside_only_one_branch.c
	modified:   test/case3_vsl_alloc_cases/18_nested_counter_alias_and_deep_rejoin.c
	modified:   test/case3_vsl_alloc_cases/19_zero_trip_then_branch_heavy_siblings.c
	modified:   test/case3_vsl_alloc_cases/20_pressure_at_deep_branch_return_all.c
	modified:   test/case3_vsl_alloc_cases/21_condition_pressure_deep_branch_dirty.c
	modified:   test/case3_vsl_alloc_cases/22_outer_body_is_branch_only.c
	modified:   test/case3_vsl_alloc_cases/23_outer_body_only_inner_loop_and_branch.c
	modified:   test/case3_vsl_alloc_cases/24_outer_dirty_through_one_inner_while.c
	modified:   test/case3_vsl_alloc_cases/25_outer_body_has_two_sibling_inner_whiles.c
	modified:   test/case3_vsl_alloc_cases/26_outer_dirty_three_loop_levels.c
	modified:   test/case3_vsl_alloc_cases/case01_loop_if_dirty.c
	modified:   test/case3_vsl_alloc_cases/case02_nested_while_if.c
	modified:   test/case3_vsl_alloc_cases/case03_nested_if_in_loop.c
	modified:   test/case3_vsl_alloc_cases/case04_zero_iteration.c
	modified:   test/case3_vsl_alloc_cases/case05_register_pressure.c
	modified:   test/case3_vsl_alloc_cases/case06_assignment_in_condition.c
	modified:   test/case3_vsl_alloc_cases/case07_three_level_loop.c
	modified:   test/case3_vsl_alloc_cases/case08_mixed_branch_merge.c
	modified:   test/case3_vsl_alloc_cases/case09_two_sequential_loops.c
	modified:   test/case3_vsl_alloc_cases/case10_many_live_values.c
	modified:   test/case3_vsl_alloc_cases/case21_loop_exit_then_post_branch.c
	modified:   test/case3_vsl_alloc_cases/case22_condition_body_counter_both_write.c
	modified:   test/case3_vsl_alloc_cases/case23_inner_loop_feeds_sibling_branch.c
	modified:   test/case3_vsl_alloc_cases/case24_zero_trip_nested_loop_after_dirty_branch.c
	modified:   test/case3_vsl_alloc_cases/case25_sibling_loops_mutate_outer_counter.c
	modified:   test/case3_vsl_alloc_cases/case26_pressure_fourteen_live_values.c
	modified:   test/case3_vsl_alloc_cases/case27_dirty_outer_values_cross_two_inner_loops.c
	modified:   test/case3_vsl_alloc_cases/case28_four_level_loop_every_level_writes.c
	modified:   test/case3_vsl_alloc_cases/case29_loop_test_value_modified_by_both_branches.c
	modified:   test/case3_vsl_alloc_cases/case30_pressure_nested_loops_branch_rejoin.c
	modified:   test/case4/1.c
	modified:   test/case4/10.c
	modified:   test/case4/11.c
	modified:   test/case4/12.c
	modified:   test/case4/13.c
	modified:   test/case4/14.c
	modified:   test/case4/15.c
	modified:   test/case4/16.c
	modified:   test/case4/17.c
	modified:   test/case4/18.c
	modified:   test/case4/2.c
	modified:   test/case4/20.c
	modified:   test/case4/21.c
	modified:   test/case4/22.c
	modified:   test/case4/25.c
	modified:   test/case4/26.c
	modified:   test/case4/27.c
	modified:   test/case4/28.c
	modified:   test/case4/29.c
	modified:   test/case4/3.c
	modified:   test/case4/30.c
	modified:   test/case4/31.c
	modified:   test/case4/32.c
	modified:   test/case4/33.c
	modified:   test/case4/34.c
	modified:   test/case4/35.c
	modified:   test/case4/36.c
	modified:   test/case4/37.c
	modified:   test/case4/38.c
	modified:   test/case4/39.c
	modified:   test/case4/4.c
	modified:   test/case4/40.c
	modified:   test/case4/41.c
	modified:   test/case4/42.c
	modified:   test/case4/43.c
	modified:   test/case4/44.c
	modified:   test/case4/45.c
	modified:   test/case4/46.c
	modified:   test/case4/47.c
	modified:   test/case4/48.c
	modified:   test/case4/49.c
	modified:   test/case4/5.c
	modified:   test/case4/50.c
	modified:   test/case4/51.c
	modified:   test/case4/52.c
	modified:   test/case4/53.c
	modified:   test/case4/54.c
	modified:   test/case4/55.c
	modified:   test/case4/56.c
	modified:   test/case4/57.c
	modified:   test/case4/58.c
	modified:   test/case4/59.c
	modified:   test/case4/6.c
	modified:   test/case4/60.c
	modified:   test/case4/61.c
	modified:   test/case4/7.c
	modified:   test/case4/8.c
	modified:   test/case4/9.c
	modified:   test/random_test/vsl_diff_fuzz.py
	modified:   x64_alloc.cpp
	modified:   x64_back_end.h

Untracked files:
  (use "git add <file>..." to include in what will be committed)
	c
	test/random_test/vsl_diff_fuzz8.py

