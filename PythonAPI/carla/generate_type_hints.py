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
                    details += f"\t@param {param['param_name']}: {param.get('doc', '')}"
                    param_sig = f"{param['param_name']}: {param.get('type', None)}"
                    if param.get('default', None):
                        param_sig += f" = {param['default']}"
                    params.append(param_sig)

                details += f"\t@return: {method.get('return', None)}"
                signature = f"{method['def_name']}({', '.join(params)}) -> {method.get('return', None)}\n\n"
                docstrings[f'{class_doc["class_name"]}.{method["def_name"]}'] = signature + details

            for instance_variable in safe_list(class_doc.get('instance_variables', [])):
                if instance_variable.get('doc', None):
                    docstrings[f'{class_doc["class_name"]}.{instance_variable["var_name"]}'] = f"{instance_variable.get('type', None)}: {instance_variable['doc']}"

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

