from inspect import currentframe


def convertToCamelCase(string):
    words = string.split(" ")
    ret = words[0].lower()
    words.pop(0)
    for w in words:
        ret += w.capitalize()
    return ret


def getStructVariableName(member):
    ret = s()
    splits = member[0].split(' ')
    if member[1]['type'] == 'select':
        ret += member[1]['select_class']
        splits.pop(0)
    else:
        ret += splits.pop(0)  # The type and a space
    return ret + ' ' + convertToCamelCase(' '.join(splits))  # The first word, lower case.


def getSetter(member, name, i, indent=2):
    ret = ''
    if member[1]['html_tag'] == 'range':
        ret += s(indent) + name + ' = (byte) server.arg(' + i + ').toInt();\n'
    if member[1]['html_tag'] == 'color':
        # First, we have to generate a char array with the 6 character html string.
        ret += s(indent) + 'server.arg(' + i + ').substring(1).toCharArray(buff, 7);\n'
        ret += s(indent) + name + ' = CRGB(strtoul(buff, NULL, 16));\n'
    if member[1]['html_tag'] == 'checkbox':
        ret += s(indent) + name + ' = server.arg(' + i + ').equals("true");\n'
    if member[1]['html_tag'] == 'select':
        ret += s(indent) + name + f' = ({member[1]["select_class"]}) server.arg(' + i + ').toInt();\n'
    return ret


def getForLoop(end, indent=2, variable="byte i", start=0):
    return f'{("    " * indent)}for({variable} = {str(start)}; i < {str(end)}; {" ".join(variable.split(" ")[1:])}++) {{ \n'


def getName(mode, vla_index):
    d = mode['um'][vla_index]
    if type(d) == list:
        d = d[0]
    return 'd.' + mode['v_name'] + '.' + convertToCamelCase(' '.join(d.split(' ')[1:]))


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
