cmake_minimum_required(VERSION 3.3 FATAL_ERROR)
foreach(p
    CMP0071 # 3.10: Let AUTOMOC and AUTOUIC process GENERATED files
    )
  if(POLICY ${p})
    cmake_policy(SET ${p} NEW)
  endif()
endforeach()

project( GraphicsView )

find_package(OpenGL)

find_package(VTK COMPONENTS
  vtkCommonCore
  vtkGUISupportQt
  vtkGUISupportQtOpenGL
  vtkIOInfovis
  vtkViewsInfovis
)
include(${VTK_USE_FILE})

if("${VTK_QT_VERSION}" STREQUAL "")
  message(FATAL_ERROR "VTK was not built with Qt")
endif()

set( Srcs
  main.cpp
  OpenGLScene.cpp
  TreeRingViewItem.cpp
  GraphLayoutViewItem.cpp
  WebView.cpp
)

set( Hdrs
  GraphicsView.hpp
  OpenGLScene.hpp
  QBoolAnimation.h
  TreeRingViewItem.h
  GraphLayoutViewItem.h
  WebView.h
)

set( MOC_Hdrs
  OpenGLScene.hpp
  QBoolAnimation.h
  WebView.h
)

set( QRCs
  GraphicsView.qrc
)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

if(VTK_QT_VERSION VERSION_GREATER "4")
  find_package(Qt5 COMPONENTS WebKitWidgets REQUIRED QUIET)

  qt5_add_resources(QRC_Srcs ${QRCs} )

  add_executable(qtgraphicsview MACOSX_BUNDLE
    ${Srcs} ${Hdrs} ${MOC_Hdrs} ${QRC_Srcs})
  qt5_use_modules(qtgraphicsview Core Gui Widgets
                  WebKit WebKitWidgets OpenGL OpenGLExtensions)
  target_link_libraries(qtgraphicsview ${VTK_LIBRARIES})
else()
  find_package(Qt4 REQUIRED)
  set(QT_USE_QTOPENGL 1)
  set(QT_USE_QTWEBKIT 1)
  include(${QT_USE_FILE})
  if (NOT QT_QTWEBKIT_FOUND OR QT_VERSION_MINOR LESS 6)
    message(STATUS "VTK isn't configured to use QtOpenGL, QtWebKit wasn't found, or Qt 4.6 wasn't found.  GraphicsView example is disabled.")
  else()
    qt4_add_resources(QRC_Srcs ${QRCs})
    qt4_wrap_cpp(MOC_Srcs ${MOC_Hdrs})

    add_executable(qtgraphicsview ${Srcs} ${Hdrs} ${MOC_Hdrs} ${QRC_Srcs})
    target_link_libraries(qtgraphicsview ${QT_LIBRARIES} ${VTK_LIBRARIES})
  endif()
endif()
