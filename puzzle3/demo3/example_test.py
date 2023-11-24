'''
Author: lianghonghua-2523 93458223+lianghonghua-2523@users.noreply.github.com 📱17727819640
Date: 2023-11-24 01:07:13
LastEditTime: 2023-11-24 01:08:04
FilePath: \demo3\example_test.py
Description: The code by lhh
Function:
'''
#!/usr/bin/env python

from __future__ import division, print_function, unicode_literals

import ttfw_idf


@ttfw_idf.idf_example_test(env_tag='Example_GENERIC', target=['esp32', 'esp32s2', 'esp32c3'], ci_target=['esp32'])
def test_examples_hello_world(env, extra_data):
    app_name = 'demo3'
    dut = env.get_dut(app_name, 'examples/get-started/hello_world')
    dut.start_app()
    res = dut.expect(ttfw_idf.MINIMUM_FREE_HEAP_SIZE_RE)
    if not res:
        raise ValueError('Maximum heap size info not found')
    ttfw_idf.print_heap_size(app_name, dut.app.config_name, dut.TARGET, res[0])


if __name__ == '__main__':
    test_examples_hello_world()
