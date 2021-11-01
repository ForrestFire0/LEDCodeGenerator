from inspect import currentframe


def convertToCamelCase(string):
    words = string.split(" ")
    ret = words[0].lower()
    words.pop(0)
    for w in words:
        ret += w.capitalize()
    return ret


def getStructVariableName(member):
    ret = ''
    if type(member) is list:
        # See if there is a VLA command, if there is a create.
        member = member[0]
    splits = member.split(' ')
    ret += s(1) + splits.pop(0) + ' '  # The type and a space
    return ret + convertToCamelCase(' '.join(splits))  # The first word, lower case.


def getSetter(input_type, name, i, indent=2):
    ret = ''
    if input_type == 'range':
        ret += s(indent) + name + ' = (byte) server.arg(' + i + ').toInt();\n'
    if input_type == 'color':
        # First, we have to generate a char array with the 6 character html string.
        ret += s(indent) + 'server.arg(' + i + ').substring(1).toCharArray(buff, 7);\n'
        ret += s(indent) + name + ' = CRGB(strtoul(buff, NULL, 16));\n'
    if input_type == 'checkbox':
        ret += s(indent) + name + ' = server.arg(' + i + ').equals("true");\n'
    if input_type == 'select':
        ret += s(indent) + name + ' = server.arg(' + i + ').toInt();\n'
    return ret


def getForLoop(end, indent=2, variable="byte i", start=0):
    return f'{("    " * indent)}for({variable} = {str(start)}; i < {str(end)}; {" ".join(variable.split(" ")[1:])}++) {{ \n'


def getName(mode, vla_index):
    d = mode['um'][vla_index]
    if type(d) == list:
        d = d[0]
    return 'd.' + mode['v_name'] + '.' + convertToCamelCase(' '.join(d.split(' ')[1:]))


def functionCall(*args):
    """
    :param args: functionName, comma seperated arguments
    :return:
    """
    args = list(args)
    ret = ""
    if len(args) < 1:
        return ret
    ret += args.pop(0) + "("
    for a in args:
        ret += str(a) + ','
    return ret[:-1] + ");"


def printl(arg):
    frame_info = currentframe()
    print(frame_info.f_back.f_lineno, ":", arg)


def s(num=1):
    return ' ' * (4 * num)


def firstLowerLetter(string):
    ret = []
    for splots in string.split(' '):
        ret.append(splots[0:1].lower())
    return ''.join(ret)
