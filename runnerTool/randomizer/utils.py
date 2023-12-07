def parse_element(element):
    result = {}

    for child in element:
        child_result = parse_element(child)

        if child.tag in result:
            if type(result[child.tag]) is list:
                result[child.tag].append(child_result)
            else:
                result[child.tag] = [result[child.tag], child_result]
        else:
            result[child.tag] = child_result

        if not result[child.tag]:
            result[child.tag] = child.text.strip() if child.text else None

        attributes = dict(child.attrib)
        if attributes:
            result[child.tag] = {'text': result[child.tag], **attributes}

    return result

def clip_value(value, lower_bound, upper_bound):
    return max(lower_bound, min(value, upper_bound))