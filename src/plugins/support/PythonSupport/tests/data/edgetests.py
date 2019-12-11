import opencor as oc
import sys

sys.dont_write_bytecode = True

from basictests import *


def edge_test_data_store_variable_index(variable, index, indent):
    print('%s          - value(%d): %f' % (indent, index, variable.value(index)))

    for run in range(variable.runs_count() + 3):
        print('%s          - value(%d, %d): %f' % (indent, index, run - 2, variable.value(index, run - 2)))


def edge_test_data_store_variable(variable, name, indent=''):
    print('%s    - Test %s:' % (indent, name))
    print('%s       - Name: %s' % (indent, variable.name()))
    print('%s       - Unit: %s' % (indent, variable.unit()))
    print('%s       - URI: %s' % (indent, variable.uri()))

    values_count = variable.values_count()

    edge_test_data_store_variable_index(variable, -1, indent)
    edge_test_data_store_variable_index(variable, 0, indent)
    edge_test_data_store_variable_index(variable, values_count - 1, indent)
    edge_test_data_store_variable_index(variable, values_count, indent)

    for run in range(variable.runs_count() + 3):
        print('%s       - values(%d): ' % (indent, run - 2), end='')
        print_values(variable.values(run - 2))


def edge_test_data_store_variables(variables, name, indent=''):
    print('%s    - Test %s:' % (indent, name))
    print('%s       - Size: %d' % (indent, len(variables)))

    for uri, variable in variables.items():
        edge_test_data_store_variable(variable, uri, indent + '   ')


if __name__ == '__main__':
    # Test for no file name or URL provided

    try:
        test_simulation('No file name or URL provided', '')
    except Exception as e:
        print(' - %s' % repr(e))

    # Test for an unknown local/remote file

    try:
        test_simulation('Unknown local file', 'unknown')
    except Exception as e:
        print(' - %s' % repr(e))

    try:
        test_simulation('Unknown remote file',
                        'https://unknown', False)
    except Exception as e:
        print(' - %s' % repr(e))

    # Test for an invalid local/remote file

    try:
        test_simulation('Invalid local file', 'cellml/underconstrained_model.cellml', False)
    except Exception as e:
        print(' - %s' % repr(e))

    try:
        test_simulation('Invalid remote file',
                        'https://raw.githubusercontent.com/opencor/opencor/master/models/tests/cellml/underconstrained_model.cellml',
                        False)
    except Exception as e:
        print(' - %s' % repr(e))

    # Test various edge cases using a valid SED-ML file

    header('Various edge cases', False)

    simulation = open_simulation('sedml/lorenz.sedml')

    print(' - Run simulation:')

    simulation.run()

    results = simulation.results()

    edge_test_data_store_variable(results.voi(), 'SimulationResults.voi()')
    edge_test_data_store_variables(results.constants(), 'SimulationResults.constants()')
    edge_test_data_store_variables(results.states(), 'SimulationResults.states()')
    edge_test_data_store_variables(results.rates(), 'SimulationResults.rates()')
    edge_test_data_store_variables(results.algebraic(), 'SimulationResults.algebraic()')

    print('    - Test SimulationResults.data_store():')

    data_store = results.data_store()

    edge_test_data_store_variable(data_store.voi(), 'DataStore.voi()', '   ')
    edge_test_data_store_variables(data_store.variables(), 'DataStore.variables()', '   ')
    edge_test_data_store_variables(data_store.voi_and_variables(), 'DataStore.voi_and_variables()', '   ')

    oc.close_simulation(simulation)
