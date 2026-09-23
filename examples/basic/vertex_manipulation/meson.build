# Each case generates its own geometry and output before checking the thrown vertices.
foreach vertex_case : [
    'position_x', 'position_y', 'position_z',
    'uniform_x', 'uniform_y', 'uniform_z',
    'gaussian_x', 'gaussian_y', 'gaussian_z', 'sphere',
    'theta_position', 'phi_position', 'uniform_theta', 'gaussian_theta', 'cosine_theta',
    'uniform_phi', 'phi_wraparound',
]
    test('examples_vertex_' + vertex_case, python_exe,
         args : [files('check_vertex.py'), gemc.full_path(), files(vertex_case + '.yaml')],
         depends : gemc,
         env : project_test_env,
         suite : 'vertex_manipulation',
         timeout : 60)
endforeach
