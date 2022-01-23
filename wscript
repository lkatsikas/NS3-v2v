# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

# def options(opt):
#     pass

# def configure(conf):
#     conf.check_nonfatal(header_name='stdint.h', define_name='HAVE_STDINT_H')

def build(bld):
    module = bld.create_ns3_module('v2v', ['core', 'internet','wave'])
    module.source = [
        'model/v2v-control-client.cc',
        'model/v2v-cluster-sap.cc',
        'model/v2v-cluster-header.cc',
        'model/v2v-mobility-model.cc',
        'helper/v2v-control-client-helper.cc'
        ]

    module_test = bld.create_ns3_module_test_library('v2v')
    module_test.source = [
        'test/v2v-test-suite.cc',
        ]

    headers = bld(features='ns3header')
    headers.module = 'v2v'
    headers.source = [
        'model/v2v-control-client.h',
        'model/v2v-cluster-header.h',
        'model/v2v-cluster-sap.h',
        'model/v2v-mobility-model.h',
        'helper/v2v-control-client-helper.h'
        ]

    if bld.env.ENABLE_EXAMPLES:
        bld.recurse('examples')

    # bld.ns3_python_bindings()

