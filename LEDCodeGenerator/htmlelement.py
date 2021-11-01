def makeHTMLE(t, **kwargs):
    """
    Returns an HTMLElement object with tag t and attributes attributes
    :param t: tag of the html element
    :param kwargs: attributes of the element
    """
    content = ''
    if 'content' in kwargs:
        content = kwargs['content']
        del kwargs['content']
    return {'tag': t, 'attributes': kwargs, 'content': content}


def stringifyE(element):
    s = ''
    for key, value in element.items():
        if key not in ['content', 'attributes', 'tag']:
            element['attributes'][key] = value

    for key, value in element['attributes'].items():
        if value != '':
            s += f' {key}="{value}"'
        else:
            s += f' {key}'
    if element["content"] is None:
        element["content"] = ''
    no_closing = ['img', 'input', 'br', 'hr', 'meta']
    if element['tag'] in no_closing:
        return f'<{element["tag"]}{s}>{element["content"]}'
    else:
        return f'<{element["tag"]}{s}>{element["content"]}</{element["tag"]}>'
