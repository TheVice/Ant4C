project('ant4c', 'c')

ant4c_sources = [
  'argument_parser.c', 'buffer.c', 'choose_task.c', 'common.c',
  'conversion.c', 'copy_move.c', 'date_time.c', 'echo.c',
  'environment.c', 'exec.c', 'fail_task.c', 'file_system.c',
  'for_each.c', 'hash.blake2.c', 'hash.blake3.c', 'hash.c',
  'hash.crc32.c', 'hash.sha3.c', 'if_task.c',
  'interpreter.c', 'listener.c', 'load_file.c', 'load_tasks.c',
  'math_unit.c', 'operating_system.c', 'path.c', 'project.c',
  'property.c', 'range.c', 'shared_object.c', 'sleep_unit.c',
  'string_unit.c', 'target.c', 'task.c', 'text_encoding.c',
  'try_catch.c', 'version.c', 'xml.c'
]

framework_gate_sources = [
  'clr_control.c', 'meta_host.c', 'runtime_host.c',
  'runtime_info.c', 'unknown_structure.c'
]

ant4c_net_framework_module_sources = [
  'ant4c.net.framework.module.c', 'host_controller.c'
]

# ant4c_tests_source = [
#   'tests_ant4c.net.framework.module.cpp', 'tests_ant4c.net.module.cpp',
#   'tests_argument_parser.cpp', 'tests_base_xml.cpp',
#   'tests_conversion.cpp', 'tests_date_time.cpp',
#   'tests_environment.cpp', 'tests_exec.cpp',
#   'tests_file_system.cpp', 'tests_hash.cpp',
#   'tests_interpreter.cpp', 'tests_load_file.cpp',
#   'tests_load_tasks.cpp', 'tests_math_unit.cpp',
#   'tests_path.cpp', 'tests_project.cpp',
#   'tests_property.cpp', 'tests_string_unit.cpp',
#   'tests_text_encoding.cpp', 'tests_xml.cpp'
# ]

ant4c_static_library = static_library('ant4c', ant4c_sources)

default_listener_dynamic_library = shared_library('default_listener', 'default_listener.c')
example_of_the_module_dynamic_library = shared_library('example_of_the_module', 'example_of_the_module.c', link_with : ant4c_static_library)

cc = meson.get_compiler('c')
m_dep = cc.find_library('m', required : false)

executable('ant4c_app', 'main.c', link_with : ant4c_static_library, dependencies : [ m_dep ])

net_gate_static_library = static_library('net_gate', 'host_fxr.c')
framework_gate_static_library = static_library('framework_gate', framework_gate_sources)

ant4c_net_module_shared_library = shared_library('ant4c.net.module', 'ant4c.net.module.c', link_with : [ net_gate_static_library, ant4c_static_library ])
ant4c_net_framework_module_shared_library = shared_library('ant4c.net.framework.module', ant4c_net_framework_module_sources, link_with : [ framework_gate_static_library, ant4c_static_library ])

tests_exec_app_exe = executable('tests_exec_app', 'tests_exec_app.c')

# dns.cpp
# regex.cpp
# text_encoding.cpp

# ant4c.net.framework.module.AssemblyInfo.cs
# ant4c.net.framework.module.CustomAppDomainManager.cs
# ant4c.net.framework.module.IFrameworkNamespace.cs
# ant4c.net.framework.module.VersionDetector.cs

# ant4c.net.module.cs
# ant4c.net.module.csproj
