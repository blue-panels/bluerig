custom_target('bluerig.1-pl',
  input: 'roff/bluerig.1.in',
  output: 'bluerig.1',
  command: [sed, man_sed_args, '@INPUT@'],
  capture: true,
  install: true,
  install_dir: mandir / 'pl' / 'man1',
)
install_symlink('brg.1', pointing_to: 'bluerig.1', install_dir: mandir / 'pl' / 'man1')
