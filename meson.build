# .

# rsync -arvu --delete --delete-excluded --exclude=_build/ unzipped/ unstable/
# rsync -arvu --delete --delete-excluded --exclude=_build/ unstable/ stage/
# rsync -arvu --delete --delete-excluded --exclude=_build/ stage/ bin2c/

# meson setup _build [--reconfigure]
# meson compile -C _build [run]

project('bin2c', 'c', version: '1.0.0.0', default_options: ['c_std=gnu11,c11', 'warning_level=3'])

IS_WIN = (build_machine.system() == 'windows') ? true : false

FS = import('fs')

USE_BZ2 = get_option('USE_BZ2')

PROGRAM_NAME     = meson.project_name()
PROGRAM_VERSION  = meson.project_version()
PROGRAM_DEFS     = []

bz2_dep = []
if USE_BZ2
  bz2_dep = dependency('bz2')
  PROGRAM_DEFS += '-DUSE_BZ2'
endif

program_executable = executable(
  meson.project_name(),
  files('bin2c.c'),
  dependencies: bz2_dep,
  c_args: PROGRAM_DEFS,
  native: true
)

run_target(
	'run',
	command: [
    program_executable
  ]
)

zip_path = find_program('7z')

project_dir = FS.parent(meson.project_build_root())

run_target(
  'zip',
  command: [
    zip_path,
    'a',
    '-t7z',
    '-xr!*_build',
    f'@PROGRAM_NAME@-@PROGRAM_VERSION@-src.7z',
    project_dir
  ]
)
