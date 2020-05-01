import glob
import re
import sys
from typing import List, Any

import yaml


def safe_list(obj: Any) -> List:
    if obj is None:
        return []
    return obj


def get_docstring(doc_file: str) -> dict:
    docstrings = dict()

    with open(doc_file, 'r') as yaml_file:
        documentations = yaml.safe_load(yaml_file)

    for documentation in documentations:
        for class_doc in documentation.get('classes', []):
            docstrings[class_doc['class_name']] = class_doc['doc']
            for method in class_doc.get('methods', []):
                params = []
                details = method.get('doc', '')
                for param in safe_list(method.get('params', [])):
                    details += "\t@param " + str(param['param_name']) + ": " + str(param.get('doc', ''))
                    param_sig = str({param['param_name']}) + ": " + str(param.get('type', None))
                    if param.get('default', None):
                        param_sig += " = " + str(param['default'])
                    params.append(param_sig)

                details += "\t@return: " + str(method.get('return', None))
                signature = str(method['def_name']) + "(" + str(', '.join(params)) + ") -> " + str(method.get('return', None)) + "\n\n"
                docstrings[str(class_doc["class_name"]) + "." + str(method["def_name"])] = signature + details

            for instance_variable in safe_list(class_doc.get('instance_variables', [])):
                if instance_variable.get('doc', None):
                    docstrings[str(class_doc["class_name"]) + "." + str(instance_variable["var_name"])] = str(instance_variable.get('type', None)) + ": " + str(instance_variable['doc'])

    return docstrings


def escape(line: str) -> str:
    return line.replace('\\', '\\\\').replace('\n', r'\n').replace('"', '\\"')


def substitute(source_file: str, docstrings: dict) -> None:
    with open(source_file) as input_file:
        lines = input_file.readlines()

    output_data = []

    pattern = re.compile(r'@DocString\((.+?)\)')
    for line in lines:
        for match in pattern.finditer(line):
            docstring = docstrings.get(match.group(1), None)
            if docstring is None:
                docstring = "no documentation available"
            line = line.replace(match.group(0), escape(docstring))
        output_data.append(line)

    with open(source_file, 'w') as output_file:
        for item in output_data:
            output_file.write(item)


if __name__ == '__main__':
    doc_files = glob.glob(sys.argv[1])  # carla/PythonAPI/docs/*.yml
    docstrings = {}
    for doc_file in doc_files:
        new_docs = get_docstring(doc_file)
        docstrings = {**docstrings, **new_docs}

    source_files = glob.glob(sys.argv[2])  # carla/PythonAPI/carla/source/libcarla_template/*.cpp
    for source_file in source_files:
        substitute(source_file, docstrings)

