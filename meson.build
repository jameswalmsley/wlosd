project('wlosd', 'cpp',
  version: '0.1',
  default_options : ['cpp_std=c++17'])

gtkmm = dependency('gtkmm-3.0')

if gtkmm.found()

  gnome = import('gnome')
  wlosd_resources = gnome.compile_resources('wlosd-resources', 'data/wlosd.gresource.xml',
    source_dir: 'data',
    c_name: 'wlosd_resources')

endif

sources = [
  wlosd_resources,
  'src/wlosd-daemon.cpp'
]

executable('wlosd-daemon',
  sources,
  dependencies: [gtkmm],
  install: true)


