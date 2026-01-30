# Notice: if both static and dynamic libraries are present, the dynamic library will be used
# Forcing suffix lib instead of variants like lib64
project('gemc', 'cpp',
        default_options : ['default_library=static', 'default_both_libraries=static', 'libdir=lib'],
        version : run_command('git', 'describe', '--tags', '--abbrev=0', '--always', check : true).stdout().strip(),
        meson_version : '>= 1.9.0'
)

# TODO: remove temp workaround for _LIBCPP_ENABLE_ASSERTIONS deprecation
add_project_arguments(
    '-U_LIBCPP_ENABLE_ASSERTIONS', # <-- Unset the deprecated macro
    '-D_LIBCPP_HARDENING_MODE_DEBUG=true',
    language : 'cpp'
)

# additional debug options useful for profiling
if get_option('buildtype') == 'debug'
    add_project_arguments('-fno-omit-frame-pointer', language : 'cpp')
    add_project_arguments('-fno-optimize-sibling-calls', language : 'cpp')
    add_project_arguments('-O2', language : 'cpp')
endif

api_dir = join_paths(get_option('prefix'), 'api')
test_env = environment()
test_env.set('PYTHONDONTWRITEBYTECODE', '1')
test_env.prepend('PYTHONPATH', api_dir)


project_description = 'GEMC (GEant Monte-Carlo) is a program based on Geant4 to simulate the passage of particles through matter. '

# init lists
############
LD = []
all_libs = []
all_includes = []
empty_dict = { 'na' : [''] }
lib_target_maps = {}
python_exe = import('python').find_installation('python3')


subdir('meson')
add_project_arguments('-Wno-shadow', language : ['cpp'])


# Fill dictionary
#################
subdir('guts')
subdir('goptions')
subdir('glogging')
subdir('gbase')
subdir('gfactory')
subdir('textProgressBar')
subdir('gtouchable')
subdir('ghit')
subdir('gtranslationTable')
subdir('gdata')
subdir('gboard')
subdir('gdynamicDigitization')
subdir('gsystem') # uses api for testing
subdir('gstreamer')
subdir('eventDispenser')
subdir('gqtbuttonswidget')
subdir('g4display')
subdir('g4dialog')
subdir('g4system')
subdir('gparticle')
subdir('gphysics')
subdir('gsplash')
subdir('gsd') # TODO: example
subdir('gfields')
subdir('gdetector')
subdir('dbselect')
subdir('gtree')
subdir('gui')
subdir('actions') # TODO: example
subdir('utilities')

# compilation loop
foreach L : LD

    this_lib_name = L['name'] # key name must be present
    sources = L.get('sources', [''])
    headers = L.get('headers', [''])
    examples = L.get('examples', empty_dict)
    geo_build = L.get('geo_build', empty_dict)
    this_deps = L.get('dependencies', [])
    this_internal_libs = L.get('internal_dependencies', [])
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
            qrc_compiled_sources += qt6.compile_resources(name : this_lib_name + '_qtresources', sources : qrc_sources)
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
            lib_target_maps += { this_lib_name : this_library }
        endif
    endif

    # header files
    if not headers.contains('')
        install_headers(headers)
    endif

    this_library_link_with = []
    # todo: this produces duplicates ROOT libraries
    if not sources.contains('')
        this_library_link_with += this_library
    endif

    foreach this_link_with : this_internal_libs
        if lib_target_maps.has_key(this_link_with)
            this_library_link_with += [lib_target_maps[this_link_with]]
        endif
    endforeach

    if plugins != empty_dict
        foreach name, sources : plugins
            shared_library(
                name,
                sources[0],
                install : sources[1],
                dependencies : [this_deps, zlib_dep],
                include_directories : all_includes + this_lib_name + additional_includes,
                link_with : this_library_link_with,
                name_suffix : 'gplugin',
                name_prefix : '',
                # Forces any static libraries that get built as part of the Meson project to be built with PIC
                # so they can be linked into a shared module
                override_options : ['b_staticpic=true']
            )

        endforeach
    endif


    # examples
    if examples != empty_dict

        # geometry, higher priority so it is run before the test
        if geo_build != empty_dict
            foreach name, geo_python_script : geo_build
                examples_dir = get_option('prefix') + '/test/' + this_lib_name
                source_subdir = this_lib_name + '/examples/'
                install_subdir(source_subdir,
                               install_dir : examples_dir,
                               strip_directory : true,
                               follow_symlinks : true)
                test(name,
                     python_exe,
                     args : [geo_python_script],
                     workdir : examples_dir,
                     is_parallel : false,
                     env : test_env,
                     priority : 20)
            endforeach
        endif

        # example executables, lower priority so it's executed after build geometry
        if not qrc_examples_sources.contains('')
            qrc_examples_compiled_sources = qt6.compile_resources(name : this_lib_name + '_qtresources', sources : qrc_examples_sources)
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
                link_with : this_library_link_with,
                # Builds the executable as a PIE (position-independent executable).
                override_options : ['b_pie=true']
            )
            test(name,
                 exe,
                 env : project_test_env,
                 args : arguments,
                 is_parallel : false,
                 priority : -20)
        endforeach
    endif
    foreach include_dir : additional_includes
        if not all_includes.contains(include_dir)
            all_includes += include_dir
        endif
    endforeach
    if not all_includes.contains(this_lib_name) and not (sources.contains('') and headers.contains(''))
        all_includes += this_lib_name
    endif
endforeach

# compile gemc
qrc_gemc_sources = qt6.compile_resources(sources : 'qtresources.qrc')

gemc_sources = ['gemc.cc', 'gemc_options.cc']
gemc = executable(
    'gemc',
    gemc_sources + qrc_gemc_sources + qrc_compiled_sources,
    install : true,
    dependencies : [yaml_cpp_dep, qt6_deps, clhep_deps, geant4_deps, ogl_deps, assimp_dep, zlib_dep],
    include_directories : all_includes,
    link_with : all_libs,
    # Builds the executable as a PIE (position-independent executable).
    override_options : ['b_pie=true']
)

# test gemc options. Adjust root to fit the geometry
examples_dir = get_option('prefix') + '/examples'
dbhost = ['-sql=' + examples_dir + '/gemc.db']

# TODO: change system with a custom with magnetic field, or use upcoming assign_field option
# If I put b2 here we need to add the diole definitions
gsystem = ['-gsystem="[{name: b2}]"']
general_gemc = ['-verbosity.gemc=2', '-debug.gemc=true', '-check_overlaps=2',
                '-verbosity.gaction=2', '-debug.gaction=true',
                '-verbosity.grunaction=2', '-debug.grunaction=true',
                '-verbosity.grun=2', '-debug.grun=true',
                '-verbosity.geventaction=2', '-debug.geventaction=true',
                '-nthreads=4', '-n=8',
                '-root="G4Box 25*cm 24*cm 40*cm G4_AIR"',
                '-gmultipoles="[{name: dipole, pole_number: 2, strength: 2, rotaxis: Z, longitudinal: true}]"',
                '-verbosity.eventdispenser=2', '-debug.eventdispenser=true']

root = ['-gstreamer="[{format: root, filename: out}]"']
ascii = ['-gstreamer="[{format: ascii, filename: out}]"']

gemc_test_sqlite = dbhost + gsystem + general_gemc

test_options = [
    { 'default' : [''] },
    { 'show_phys' : ['-showPhysics'] },
    { 'phys_verbose' : ['-verbosity.gphysics=2', '-debug.gphysics=true'] },
    { 'gui_verbose' : ['-verbosity.dbselect=2', '-debug.dbselect=true',
                       '-verbosity.gsplash=2', '-debug.gsplash=true'] },
    { 'detector_verbose' : ['-verbosity.gdetector=2', '-debug.gdetector=true',
                            '-verbosity.gsystem=2', '-debug.gsystem=true',
                            '-verbosity.g4system=2', '-debug.g4system=true'] },
    { 'gdigitization_verbose' : ['-verbosity.gdigitization=2', '-debug.gdigitization=true',
                                 '-verbosity.plugins=2', '-debug.plugins=true',
                                 '-verbosity.gtranslationtable=2', '-debug.gtranslationtable=true',
                                 '-verbosity.gsd=2', '-debug.gsd=true',
                                 '-verbosity.gdetector=2', '-debug.gdetector=true',
                                 '-verbosity.gtouchable=2', '-debug.gtouchable=true'] },
    { 'gstreamer_verbose' : ['-verbosity.digitized_data=2', '-debug.digitized_data=true',
                             '-verbosity.gstreamer=2', '-debug.gstreamer=true',
                             '-verbosity.true_data=2', '-debug.true_data=true',
                             '-gstreamer="[{format: root, filename: out}, {format: ascii, filename: out} ]"'] },
    { 'gfield_verbose' : ['-verbosity.gfield=1', '-debug.gfield=true',
                          '-gmultipoles="[{name: dipole, pole_number: 2, strength: 2, rotaxis: Z, longitudinal: true}]"',
                          '-gparticle="[{name: e-, p: 2300, theta: 23.0}]"'] }
]

foreach opt_dict : test_options
    foreach key, value : opt_dict            # each dict has exactly one entry
        test_name = 'gemc_' + key            # => gemc_default, gemc_show_pys
        test_arguments = value
        test(
            test_name,
            gemc,
            args : test_arguments + gemc_test_sqlite,
            # suite : 'gemc',
            is_parallel : false,
            timeout : 60, # adjust as needed
            priority : -20
        )
    endforeach
endforeach


subdir('api')
subdir('examples')

post_install = run_command('echo', 'Installation completed!', check : true)

# generate pkg-config file
# needs testing - add libraries?
#pkg = import('pkgconfig')
#pkg.generate(
#    name : meson.project_name(),
#    description : project_description,
#    requires : ['yaml-cpp', 'qt6', 'clhep', 'geant4'], # pkg-config dependencies only
#    version : meson.project_version(),
#    #    libraries : all_libs
#)
