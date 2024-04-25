# qt: pkg-config. https://mesonbuild.com/Qt5-module.html
qt5 = import('qt5')
qt5_deps = dependency('qt5', modules : ['Core', 'Gui', 'Widgets'])

# clhep: pkg-config
clhep_deps = dependency('clhep', version : '>=2.4.7.1')

# geant4: no pkg-config. The cmake dependency include clhep and a lot of stuff that is not working
# notice: this assumes that the geant4-config exists and is in the PATH
# geant4_deps = dependency('Geant4', version : '>=11.2.0',
#                         modules : ['Geant4::G4zlib', 'Geant4::G4tools', 'Geant4::G4ptl', 'Geant4::G4analysis', 'Geant4::G4digits_hits', 'Geant4::G4error_propagation', 'Geant4::G4event', 'Geant4::G4geometry', 'Geant4::G4global', 'Geant4::G4graphics_reps', 'Geant4::G4intercoms', 'Geant4::G4interfaces', 'Geant4::G4materials', 'Geant4::G4parmodels', 'Geant4::G4particles', 'Geant4::G4geomtext', 'Geant4::G4mctruth', 'Geant4::G4gdml', 'Geant4::G4physicslists', 'Geant4::G4processes', 'Geant4::G4readout', 'Geant4::G4run', 'Geant4::G4track', 'Geant4::G4tracking', 'Geant4::G4FR', 'Geant4::G4visHepRep', 'Geant4::G4RayTracer', 'Geant4::G4Tree', 'Geant4::G4VRML', 'Geant4::G4GMocren', 'Geant4::G4vis_management', 'Geant4::G4modeling', 'Geant4::G4ToolsSG', 'Geant4::G4OpenGL', 'Geant4::G4visQt3D'],
#                         required : true)

# setting things up manually instead
Geant4_inc_dirs = []
Geant4_flags = []
g4_cflags = run_command('geant4-config', '--cflags', check : true).stdout().strip().split()
# keep only the geant4 include directories
foreach cflag : g4_cflags
    if cflag.contains('geant4') or cflag.contains('/OpenGL')
        Geant4_inc_dirs += include_directories(cflag.strip('-I'))
    elif cflag.contains('-D') or cflag.contains('-W')
        Geant4_flags += cflag
    endif
endforeach
G4_link_args = []
Geant4_link_args = run_command('geant4-config', '--libs', check : true).stdout().strip().split()
foreach link_arg : Geant4_link_args
    if link_arg.contains('geant4') or link_arg.contains('G4')
        G4_link_args += link_arg
    endif
endforeach
geant4_deps = declare_dependency(
    include_directories : Geant4_inc_dirs,
    link_args : G4_link_args,
    compile_args : Geant4_flags
)

# sqlite
sqlite_dep = dependency('sqlite3')


# ROOT dependencies coming from cmake are not correctly handled by meson or are just plain wrong
# Doing this manually
root_cmake = dependency('root', version : '>=6.28.04')

if get_option('use_root')

    if root_cmake.found()
        ROOT_link_args = []

        ROOT_inc_dirs = include_directories(run_command('root-config', '--incdir', check : true).stdout().strip())
        ROOT_config_link_args = run_command('root-config', '--libs', check : true).stdout().strip().split()
        foreach link_arg : ROOT_config_link_args
            if link_arg.contains('root') or link_arg.contains('Core') or link_arg.contains('RIO') or link_arg.contains('Tree')
                ROOT_link_args += link_arg
            endif
        endforeach

        root_deps = declare_dependency(
            include_directories : ROOT_inc_dirs,
            link_args : ROOT_link_args,
        )
    endif
else
    root_deps = dependency('dummy-root-dep', required : false)
endif
