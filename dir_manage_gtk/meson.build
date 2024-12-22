project('system-programming', 'c')

# GTK4 의존성
gtkdep = dependency('gtk4')

# GNOME 리소스 컴파일
gnome = import('gnome')
resources = gnome.compile_resources('resources', 'column.gresource.xml')

# 프론트엔드 소스 파일
frontend_sources = files(
  'main.c', 
  'ui_list.c', 
  'ui_cd.c', 
  'ui_manage.c', 
  'communication.c', 
  'ui_link.c'
)

# 백엔드 소스 파일
backend_sources = files('backend.c')

# 프론트엔드 실행 파일
executable('column', 
  frontend_sources, 
  resources, 
  dependencies: gtkdep, 
  export_dynamic: true, 
  install: false
)

# 백엔드 실행 파일
executable('a.out', 
  backend_sources, 
  dependencies: gtkdep, 
  install: false
)
