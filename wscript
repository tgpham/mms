## -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

def build(bld):
    module = bld.create_ns3_module('mms', ['applications', 'internet'])
    module.source = [
        'model/mms-adapt-server.cc',
        'model/mms-server.cc',
        'model/mms-adapt-client.cc',
        'model/mms-client.cc',
        'model/cotp-client.cc',
        'model/cotp-server.cc',
        'model/mms-header.cc',
        'helper/mms-client-helper.cc',
        'helper/mms-server-helper.cc',
        ]

   ## module_test = bld.create_ns3_module_test_library('mms')
   ## module_test.source = [
   ##     'test/mms-client-server-test.cc',
   ##     ]

    headers = bld.new_task_gen(features=['ns3header'])
    headers.module = 'mms'
    headers.source = [
        'model/mms-adapt-server.h',
        'model/mms-server.h',
        'model/mms-adapt-client.h',
        'model/mms-client.h',
        'model/cotp-client.h',
        'model/cotp-server.h',
        'model/mms-header.h',
        'helper/mms-client-helper.h',
        'helper/mms-server-helper.h',
        ]

   # bld.ns3_python_bindings()
