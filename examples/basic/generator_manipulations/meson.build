# Each case generates its own geometry and output before checking the thrown vertices.
foreach generator_case : [
    'uniform_z', 'uniform_xy', 'gaussian_z', 'gaussian_xy', 'sphere',
    'fixed_theta_and_phi', 'uniform_theta', 'gaussian_theta', 'cosine_theta',
    'uniform_phi', 'phi_wraparound',
]
    test('examples_generator_' + generator_case, python_exe,
         args : [files('check_generator.py'), gemc.full_path(), files(generator_case + '.yaml')],
         depends : gemc,
         env : project_test_env,
         suite : 'generator_manipulations',
         timeout : 60)
endforeach
