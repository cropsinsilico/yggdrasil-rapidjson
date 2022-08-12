import copy
import json
import argparse
import os
from urllib.request import urlopen


def create_full_schema(fname):
    import yaml
    from yggdrasil import schema
    s = schema.get_schema()
    s.save(fname, schema=s.full_schema)


def get_ygg_tests():
    import yaml
    fname = os.path.join(os.path.dirname(__file__),
                         'test', 'full_schema.yml')
    if args.create_full_schema or not os.path.isfile(fname):
        try:
            create_full_schema(fname)
        except ImportError:
            return []
    with open(fname, 'r') as fd:
        base = yaml.load(fd, yaml.SafeLoader)
    try:
        base['definitions']['file']['allOf'][0]['properties']['name']['pattern'] = base['definitions']['file']['allOf'][0]['properties']['name']['pattern'].replace('\\', '\\\\')
    except KeyError:
        pass
    try:
        base['definitions']['file']['allOf'][1]['anyOf'][0]['properties']['name']['pattern'] = base['definitions']['file']['allOf'][1]['anyOf'][0]['properties']['name']['pattern'].replace('\\', '\\\\')
    except KeyError:
        pass
    test_yaml = (
        {'models': [
            {'name': 'modelA',
             'language': 'python',
             'args': ['model.py', '-v'],
             'outputs': [
                 {'name': 'outputA',
                  'column_names': ['a', 'b'],
                  'column_units': ['cm', 'g'],
                  'filter': {
                     'function': 'example_python:example_filter'}}],
            'working_dir': os.getcwd()},
            {'name': 'modelB',
             'language': 'c',
             'args': './src/modelA.c',
             'function': 'fake',
             'is_server': {'input': 'A', 'output': 'B'},
             'outputs': 'B',
             'working_dir': os.getcwd()},
            {'args': './src/modelA.c',
             'driver': 'GCCModelDriver',
             'inputs': ['inputA'],
             'name': 'modelA',
             'outputs': ['outputA']}],
            # {'name': 'modelA',
            #  'args': './src/modelA.c',
            #  'driver': 'GCCModelDriver',
            #  'inputs': [{'name': 'inputA',
            #              'args': '/var/folders/6y/tnvg4kjn4n72pcpqw__8jjmh0000gn/T/tmp_5729eb32_0.yml',
            #              'driver': 'FileInputDriver',
            #              'onexit': 'printStatus',
            #              'translator': 'tests.test_yamlfile:direct_translate'}],
            #  'outputs': [{'name': 'outputA',
            #               'driver': 'FileOutputDriver',
            #               'args': 'fileA.txt',
            #               'onexit': 'printStatus',
            #               'translator': 'tests.test_yamlfile:direct_translate'},
            #              {'name': 'outputA2',
            #               'driver': 'OutputDriver',
            #               'args': 'A_to_B',
            #               'onexit': 'printStatus',
            #               'translator': 'tests.test_yamlfile:direct_translate'}]}],
         'connections': [
             {'input': 'outputA',
              'output': 'fileA.txt',
              'seritype': 'ply'},
             {'inputs': ['/var/folders/6y/tnvg4kjn4n72pcpqw__8jjmh0000gn/T/tmp_7c25e645_0.yml'],
              'outputs': 'inputA',
              'read_meth': 'all'},
             {'input': 'outputA',
              'output': ['output.txt'],
              'write_meth': 'all'}],
         'working_dir': os.getcwd()},
        {'models': [
            {'name': 'modelA',
             'language': 'python',
             'args': ['model.py', '-v'],
             'inputs': [{'commtype': 'default',
                         'datatype': '-YGG-eyJ0eXBlIjoic2NoZW1hIn0=-YGG-eyJ0eXBlIjoiYnl0ZXMifQ==-YGG-',
                         'is_default': True,
                         'name': 'input'}],
             'outputs': [{'name': 'outputA',
                          'commtype': 'default',
                          'datatype': '-YGG-eyJ0eXBlIjoic2NoZW1hIn0=-YGG-eyJ0eXBlIjoiYnl0ZXMifQ==-YGG-',
                          'filter': {
                              'function': '-YGG-eyJ0eXBlIjoiY2xhc3MifQ==-YGG-ZXhhbXBsZV9weXRob246ZXhhbXBsZV9maWx0ZXI=-YGG-'},
                          'field_names': ['a', 'b'],
                          'field_units': ['cm', 'g']}],
             'working_dir': os.getcwd()},
            {'name': 'modelB',
             'language': 'c',
             'args': ['./src/modelA.c'],
             'function': 'fake',
             'is_server': {'input': 'A', 'output': 'B'},
             'inputs': [{'commtype': 'default',
                         'datatype': '-YGG-eyJ0eXBlIjoic2NoZW1hIn0=-YGG-eyJ0eXBlIjoiYnl0ZXMifQ==-YGG-',
                         'is_default': True,
                         'name': 'input'}],
             'outputs': [{'name': 'B',
                          'commtype': 'default',
                          'datatype': '-YGG-eyJ0eXBlIjoic2NoZW1hIn0=-YGG-eyJ0eXBlIjoiYnl0ZXMifQ==-YGG-'}],
             'working_dir': os.getcwd()},
            {'args': ['./src/modelA.c'],
             'driver': 'GCCModelDriver',
             'inputs': [{'name': 'inputA',
                         'commtype': 'default',
                         'datatype': '-YGG-eyJ0eXBlIjoic2NoZW1hIn0=-YGG-eyJ0eXBlIjoiYnl0ZXMifQ==-YGG-'}],
             'name': 'modelA',
             'language': 'executable',
             'outputs': [{'name': 'outputA',
                          'commtype': 'default',
                          'datatype': '-YGG-eyJ0eXBlIjoic2NoZW1hIn0=-YGG-eyJ0eXBlIjoiYnl0ZXMifQ==-YGG-'}],
             'working_dir': os.getcwd()}],
            # {'name': 'modelA',
            #  'args': ['./src/modelA.c'],
            #  'driver': 'GCCModelDriver',
            #  'language': 'executable',
            #  'working_dir': os.getcwd(),
            #  'inputs': [{'name': 'inputA',
            #              'args': '/var/folders/6y/tnvg4kjn4n72pcpqw__8jjmh0000gn/T/tmp_5729eb32_0.yml',
            #              'driver': 'FileInputDriver',
            #              'onexit': 'printStatus',
            #              'transform': ['-YGG-eyJ0eXBlIjoiY2xhc3MifQ==-YGG-dGVzdHMudGVzdF95YW1sZmlsZTpkaXJlY3RfdHJhbnNsYXRl-YGG-'],
            #              'datatype': '-YGG-eyJ0eXBlIjoic2NoZW1hIn0=-YGG-eyJ0eXBlIjoiYnl0ZXMifQ==-YGG-',
            #              'commtype': 'default'}],
            #  'outputs': [{'name': 'outputA',
            #               'driver': 'FileOutputDriver',
            #               'args': 'fileA.txt',
            #               'onexit': 'printStatus',
            #               'transform': ['-YGG-eyJ0eXBlIjoiY2xhc3MifQ==-YGG-dGVzdHMudGVzdF95YW1sZmlsZTpkaXJlY3RfdHJhbnNsYXRl-YGG-'],
            #               'datatype': '-YGG-eyJ0eXBlIjoic2NoZW1hIn0=-YGG-eyJ0eXBlIjoiYnl0ZXMifQ==-YGG-',
            #               'commtype': 'default'},
            #              {'name': 'outputA2',
            #               'driver': 'OutputDriver',
            #               'args': 'A_to_B',
            #               'onexit': 'printStatus',
            #               'transform': ['-YGG-eyJ0eXBlIjoiY2xhc3MifQ==-YGG-dGVzdHMudGVzdF95YW1sZmlsZTpkaXJlY3RfdHJhbnNsYXRl-YGG-'],
            #               'datatype': '-YGG-eyJ0eXBlIjoic2NoZW1hIn0=-YGG-eyJ0eXBlIjoiYnl0ZXMifQ==-YGG-',
            #               'commtype': 'default'}]}],
         'connections': [
             {'inputs': [
                 {'name': 'outputA',
                  'datatype': '-YGG-eyJ0eXBlIjoic2NoZW1hIn0=-YGG-eyJ0eXBlIjoiYnl0ZXMifQ==-YGG-',
                  'commtype': 'default',
                  'working_dir': os.getcwd()}],
              'outputs': [
                  {'name': 'fileA.txt',
                   'filetype': 'binary',
                   'serializer': {'seritype': 'ply'},
                   'working_dir': os.getcwd()}],
              'working_dir': os.getcwd()},
             {'inputs': [{'name': '/var/folders/6y/tnvg4kjn4n72pcpqw__8jjmh0000gn/T/tmp_7c25e645_0.yml',
                          'filetype': 'binary',
                          'working_dir': os.getcwd()}],
              'outputs': [{'name': 'inputA',
                           'commtype': 'default',
                           'datatype': '-YGG-eyJ0eXBlIjoic2NoZW1hIn0=-YGG-eyJ0eXBlIjoiYnl0ZXMifQ==-YGG-'}],
              'working_dir': os.getcwd(),
              'read_meth': 'all'},
             {'inputs': [{'name': 'outputA',
                          'commtype': 'default',
                          'datatype': '-YGG-eyJ0eXBlIjoic2NoZW1hIn0=-YGG-eyJ0eXBlIjoiYnl0ZXMifQ==-YGG-',
                          'working_dir': os.getcwd()}],
              'outputs': [{'name': 'output.txt',
                           'filetype': 'binary',
                           'serializer': {'seritype': 'default'},
                           'working_dir': os.getcwd()}],
              'working_dir': os.getcwd(),
              'write_meth': 'all'}],
         'working_dir': os.getcwd()})
    return (["#define METASCHEMA_YGG_TESTS", ""]
            + make_function("get_yggschema", base)
            + make_function("get_testschema", test_yaml[0])
            + make_function("get_testschema_result", test_yaml[1]))


def make_function(name, base):
    return [
        "template<typename T>",
        "inline const typename item_return<T>::type* " + name + "() { return nullptr; }", "",
        "template<>",
        "inline const item_return<char>::type* " + name + "<char>() {",
        "  const char* out = \""
        + json.dumps(base, indent=1).replace(
            "\"\\t\"", "\"\\\\t\"").replace(
            "\"\\n\"", "\"\\\\n\"").replace(
            "\\\"", "\\\\\"").replace(
            "\"", "\\\"").replace(
            '\n', "\"\n    \"")
        + "\";",
        "  return out;",
        "}", "",
        "template<>",
        "inline const item_return<wchar_t>::type* " + name + "<wchar_t>() {",
        "  const wchar_t* out = L\""
        + json.dumps(base, indent=1).replace(
            "\"\\t\"", "\"\\\\t\"").replace(
            "\"\\n\"", "\"\\\\n\"").replace(
            "\\\"", "\\\\\"").replace(
            "\"", "\\\"").replace(
            '\n', "\"\n    L\"")
        + "\";",
        "  return out;",
        "}", ""]


if __name__ == "__main__":
    parser = argparse.ArgumentParser("Create a C++ header file containing "
                                     "the YGGDRASIL schema")
    parser.add_argument("dest",
                        help=("Path to header file where the metaschema "
                              "should be saved."))
    parser.add_argument("--base-draft", default="draft-04",
                        help="JSON schema draft that should be used as a base")
    parser.add_argument("--ygg-tests", action='store_true',
                        help='Include tests for the yggdrasil schema')
    parser.add_argument("--create-full-schema", action='store_true',
                        help='Create a full_schema.yml file')
    args = parser.parse_args()
    url = f'http://json-schema.org/{args.base_draft}/schema#'
    standard = json.loads(urlopen(url).read().decode('utf-8'))
    base = copy.deepcopy(standard)
    base['title'] = "Ygg meta-schema for data type schemas"
    base['definitions']['simpleTypes']['enum'] += [
        "1darray", "any", "bytes", "class", "complex", "float",
        "function", "instance", "int", "ndarray", "obj", "ply",
        "scalar", "schema", "uint", "unicode"]
    base['properties'].update({
        "args": {
            "description": "Arguments required to recreate a class instance.",
            "type": "array"
        },
        "class": {
            "anyOf": [
                {"type": "class"},
                {"items": {"type": "class"},
                 "minItems": 1,
                 "type": "array"}],
            "description": "One or more classes that the object should be an instance of."
        },
        "kwargs": {
            "description": "Keyword arguments required to recreate a class instance.",
            "type": "object"
        },
        "length": {
            "description": "Number of elements in the 1D array.",
            "minimum": 1,
            "type": "number"
        },
        "precision": {
            "description": "The size (in bits) of each item.",
            "minimum": 1,
            "type": "number"
        },
        "shape": {
            "description": "Shape of the ND array in each dimension.",
            "items": {
                "minimum": 1,
                "type": "integer"
            },
            "type": "array"
        },
        "subtype": {
            "description": "The base type for each item.",
            "enum": [
                "bytes",
                "complex",
                "float",
                "int",
                "uint",
                "unicode"
            ],
            "type": "string"
        },
        "units": {
            "description": "Physical units.",
            "type": "string"
        },
        "aliases": {
            "description": "Aliases for a property that also be used.",
            "type": "array",
            "items": {"type": "string"}
        },
        "allowSingular": {
            "description": "If true, the value may only contain an element matching the schema for 1) all array items, 2) the only array item in a 1-element long array, 3) the first required object property, 4) the only object property in a 1-element long object. Only valid for array & object schemas.",
            "type": ["boolean", "string"],
            "default": False
        },
        "deprecated": {
            "description": ("Message about the deprecation of a schema property that will be displayed during validation."
                            " If true, a generic warning will be displayed."),
            "type": ["boolean", "string"],
        },
        "pullProperties": {
            "description": "Pull properties from another location in the provided JSON document. If true, any missing local properties will be pulled from the parent object. If an array of property names is provided, only those local properties in the array will be pulled from the parent object. If an object is provided, the keys should be relative or absolute paths to objects in the JSON document that properties will be pulled from with the values specifying which properties should be pulled (true for all properties and an array or a select subset).",
            "oneOf": [
                {"type": "boolean"},
                {"type": "array",
                 "items": {"type": "string"}},
                {"type": "object",
                 "additionalProperties": {
                     "oneOf": [
                         {"type": "boolean"},
                         {"type": "array",
                          "items": {"type": "string"}}]}}],
            "default": False
        },
        "pushProperties": {
            "description": "Push properties to another location in the provided JSON document. If true, any properties missing from the parent will be pushed to the parent object. If an array of property names is provided, only those parent properties in the array will be pushed to the parent object. If an object is provided, the keys should be relative or absolute paths to objects in the JSON document that properties will be pushed to with the values specifying which properties should be pushed (true for all missing destination properties and an array or a select subset).",
            "oneOf": [
                {"type": "boolean"},
                {"type": "array",
                 "items": {"type": "string"}},
                {"type": "object",
                 "additionalProperties": {
                     "oneOf": [
                         {"type": "boolean"},
                         {"type": "array",
                          "items": {"type": "string"}}]}}],
            "default": False
        }
    })
    contents = ["// This file is generated by create_metaschema.py do not modify directly", "",
                "#ifndef METASCHEMA_H_",
                "#define METASCHEMA_H_", "",
                "template<class T>",
                "struct item_return{ typedef T type; };", ""]
    contents += (make_function("get_metaschema", base)
                 + make_function("get_standard_metaschema", standard))
    ## Create test
    if args.ygg_tests:
        contents += get_ygg_tests()
    ## End test
    contents += ["", "#endif // METASCHEMA_H_", ""]
    with open(args.dest, 'w') as fd:
        fd.write('\n'.join(contents))
        
