foreach name : ['complete_engine', 'hotkey_equal', 'group_init_destroy', 'group_set_state',
                'scrollbar', 'table_datasource', 'textarea', 'widget_find_by_id',
                'widget_make_global_local', 'paste', 'table_quick_search', 'filechooser',
                'buttonbar_modifiers', 'input_edit', 'try_complete']
  test(name,
    executable(name, name + '.c',
      c_args: test_c_args,
      dependencies: test_lib_deps),
    workdir: meson.current_build_dir(),
    suite: 'lib/widget')
endforeach

# the widgets drawn on the virtual screen of S-Lang, on a pseudo-terminal of the test
if get_option('screen') == 'slang' and have_terminal
  foreach name : ['screen_dialog']
    test(name,
      executable(name, name + '.c',
        c_args: test_c_args + [
          '-DTEST_SKIN_DIR="@0@"'.format(meson.project_source_root() / 'misc' / 'skins'),
        ],
        dependencies: [test_src_deps, terminal_dep]),
      workdir: meson.current_build_dir(),
      suite: 'lib/widget')
  endforeach
endif
