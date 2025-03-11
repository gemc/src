# Notice: if both static and dynamic libraries are present, the dynamic library will be used
project('gemc3', 'cpp',
        default_options : ['cpp_std=c++20', 'default_library=static', 'default_both_libraries=static'],
        version : run_command('git', 'describe', '--tags', '--abbrev=0', '--always', check : true).stdout().strip(),
        meson_version : '>= 1.6.0'
)
project_description = 'GEMC (GEant Monte-Carlo) is a program based on Geant4 to simulate the passage of particles through matter. '

# init lists
############
LD = []
all_libs = []
all_includes = []
empty_dict = { 'na' : [''] }
all_lib_names = []  # Store library names as linker flags

subdir('meson')
add_project_arguments('-Wno-shadow', language : ['cpp'])

# Fill dictionary
#################
subdir('guts')
subdir('goptions')
subdir('gfactory')
subdir('textProgressBar')
subdir('gtouchable')
subdir('ghit')
subdir('gtranslationTable')
subdir('gdata')
subdir('gdynamicDigitization')
subdir('eventDispenser')
subdir('gQtButtonsWidget')
subdir('g4display')
subdir('g4dialog')
subdir('gsystem')
subdir('g4system')
subdir('gparticle')
subdir('gphysics')
subdir('gsplash')
subdir('gstreamer')
subdir('gsession')
subdir('gsd')
subdir('gfields')
subdir('utilities')
subdir('gdetector')
subdir('userActions')
subdir('gui')

# compilation loop
foreach L : LD

    this_lib_name = L['name'] # key name must be present

    sources = L.get('sources', [''])
    headers = L.get('headers', [''])
    examples = L.get('examples', empty_dict)
    this_deps = L.get('dependencies', [])
    plugins = L.get('plugins', empty_dict)
    additional_includes = L.get('additional_includes', [''])
    moc_headers = L.get('moc_headers', [''])
    moc_sources = []
    qrc_sources = L.get('qrc_sources', [''])
    qrc_examples_sources = L.get('qrc_examples_sources', [''])
    qrc_compiled_sources = []
    qrc_examples_compiled_sources = []

    if not moc_headers.contains('')
        moc_sources = qt6.compile_moc(headers : moc_headers,
                                      dependencies : this_deps)
    endif

    if not sources.contains('')
        if not qrc_sources.contains('')
            qrc_compiled_sources += qt6.compile_resources(sources : qrc_sources)
        endif
        this_library = static_library(
            this_lib_name,
            sources + moc_sources + qrc_compiled_sources,
            install : true,
            dependencies : this_deps,
            include_directories : all_includes + additional_includes
        )
        if not all_libs.contains(this_library)
            all_libs += this_library
            all_lib_names += 'lib' + this_lib_name + '.a' # Add as linker flag
            message(' Adding Library ' + this_lib_name)
        endif
    endif

    # header files
    if not headers.contains('')
        install_headers(headers)
    endif

    #  plugins
    #  explicitly tell the linker to prefer dynamic libz.so with link_args below
#    if host_machine.system() == 'darwin'
#        # macOS: Do not use -Bdynamic, it's not needed
#        zlib_link_args = ['-lz']
#    else
#        # Linux: Use -Bdynamic
#        zlib_link_args = ['-Wl,-Bdynamic', '-lz']
#    endif

    if plugins != empty_dict
        foreach name, sources : plugins
            shared_library(
                name,
                sources[0],
                install : sources[1],
                dependencies : [this_deps, zlib_dep],
                include_directories : all_includes + this_lib_name + additional_includes,
                link_with : all_libs,
                name_suffix : 'gplugin',
                name_prefix : '',
                override_options: ['b_staticpic=false']
                # link_args : all_lib_names
            )
        endforeach
    endif

    # examples
    if examples != empty_dict
        if not qrc_examples_sources.contains('')
            qrc_examples_compiled_sources = qt6.compile_resources(sources : qrc_examples_sources)
        endif
        foreach name, sources_and_arguments : examples
            # concatenate the items in sources_and_arguments[1] to a string
            example_sources = sources_and_arguments[0] + qrc_examples_compiled_sources
            arguments = sources_and_arguments[1]

            exe = executable(
                name,
                example_sources,
                install : true,
                dependencies : this_deps,
                include_directories : all_includes + this_lib_name + additional_includes,
                link_with : all_libs,
                #link_args : all_lib_names
                override_options: ['b_pie=true']
            )
            test(' ## ' + this_lib_name + ' test ##  >>  ' + name,
                 exe,
                 env : project_test_env,
                 args : arguments)
        endforeach
    endif
    foreach include_dir : additional_includes
        if not all_includes.contains(include_dir)
            all_includes += include_dir
        endif
    endforeach
    if not all_includes.contains(this_lib_name) and not sources.contains('')
        all_includes += this_lib_name
    endif
endforeach

# compile gemc
qrc_gemc_sources = qt6.compile_resources(sources : 'qtresources.qrc')

gemc = executable(
    'gemc',
    ['gemc.cc'] + qrc_gemc_sources + qrc_compiled_sources,
    install : true,
    dependencies : [yaml_cpp_dep, qt6_deps, clhep_deps, geant4_deps, ogl_deps, sqlite_dep, expat_dep, assimp_dep],
    include_directories : all_includes,
    # link_with : all_libs
    link_args : all_lib_names,
    override_options: ['b_pie=true']
)

# api
subdir('api')
subdir('examples')

# generate pkg-config file
# needs testing - add libraries?
pkg = import('pkgconfig')
pkg.generate(
    name : meson.project_name(),
    description : project_description,
    requires : ['yaml-cpp', 'qt6', 'clhep', 'geant4'], # pkg-config dependencies only
    version : meson.project_version(),
    #    libraries : all_libs
    #    link_args : all_lib_names
)
