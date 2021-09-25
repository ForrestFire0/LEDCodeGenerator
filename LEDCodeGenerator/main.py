import json
import os

debug_setters = False
update_temp = True


def convertToCamelCase(string):
    words = string.split(" ")
    ret = words[0].lower()
    words.pop(0)
    for w in words:
        ret += w.capitalize()
    return ret


def generateDataUnion():
    ret = "union Data {\n"

    for mode in data['modes']:
        ret += '     ' + mode['classname'] + "Data " + mode['v_name'] + ';\n'
    ret += '} d;\n\n'
    return ret


def generateStructs():
    ret = ""
    for mode in data['modes']:
        ret += 'struct ' + mode['classname'] + 'Data {\n'
        if mode.get('um'):
            for member in mode.get('um'):
                splits = member.split(' ')
                ret += '     ' + splits.pop(0) + ' '  # The type and a space
                ret += convertToCamelCase(' '.join(splits))  # The first word, lower case.
                ret += ';\n'
        if mode.get('im'):
            for member in mode.get('im'):
                splits = member.split(' ')
                ret += '     ' + splits.pop(0) + ' '  # The type and a space
                ret += convertToCamelCase(' '.join(splits))  # The first word, lower case.
                ret += ';\n'
        ret += '};\n\n'
    return ret


def getInputFromType(ty):
    return {
        "CRGB": "color",
        "byte": "range",
        "bool": "checkbox",
        "boolean": "checkbox"
    }[ty]


def generateHTMLCode():
    ret = 'const inputs = {\n'
    for mode in data['modes']:
        ret += '     \'' + mode['name'] + '\': [\''
        i = 0
        if mode.get('um'):
            for member in mode.get('um'):
                splits = member.split(' ')
                ty = splits.pop(0)  # The type and a space
                input_type = getInputFromType(ty)
                ret += ' '.join(splits) + ': <input type=\"' + input_type + '\" id=\"' + str(i) + '\"'
                if input_type == 'range':
                    if mode.get('max') and mode.get('max')[i]:
                        ret += ' max="' + str(mode.get('max')[i]) + '"'
                    else:
                        ret += ' max="255"'
                try:
                    if input_type == 'color' or input_type == 'range':
                        ret += ' value="' + str(mode['default'][i]) + '"'
                    if input_type == 'checkbox':
                        if mode['default'][i]:
                            ret += 'checked'
                except (KeyError, IndexError):
                    print("Fail: Unmatched um item with default parameter for mode " + mode['name'])
                    exit(-1)
                ret += ' oninput="u()"><br>'
                i = i + 1
        ret += '\',' + str(len(mode.get('um') if mode.get('um') else [])) + '],\n'

    return ret + '};\n'


# This function will generate the code required to take the encoded parameters in order and set variables.
# For example, for dot, a color should be send back and set equal to the variable.
def generateSettersCode():
    ret = 'void fillInArgs(Mode selected, ESP8266WebServer &server) {\n    char buff[7];\n    switch (selected) {\n'
    for mode in data['modes']:
        ret += '     case ' + mode['enum_name'] + ':\n'
        i = 0
        if mode.get('um'):
            for member in mode.get('um'):
                splits = member.split(' ')
                ty = splits.pop(0)  # The type and a space
                input_type = getInputFromType(ty)
                name = 'd.' + mode['v_name'] + '.' + convertToCamelCase(' '.join(splits))
                if input_type == "range":
                    ret += '        ' + name + ' = (byte) server.arg(' + str(i + 1) + ').toInt();\n'
                if input_type == "color":
                    # First, we have to generate a char array with the 6 character html string.
                    ret += '        server.arg(' + str(i + 1) + ').substring(1).toCharArray(buff, 7);\n'
                    ret += '        ' + name + ' = CRGB(strtoul(buff, NULL, 16));\n'
                if input_type == 'checkbox':
                    ret += '        ' + name + ' = server.arg(' + str(i + 1) + ').equals("true");\n'
                if debug_setters:
                    ret += '        Serial.print("' + name + ': ");\n        ' + 'Serial.println(' + name + ');\n'
                i = i + 1
        ret += '        break;\n'
    ret += '     }\n}\n\n'
    return ret


def generateLEDOptions():
    ret = 'char *LEDOptions[] = {'
    for mode in data['modes']:
        ret += '"' + mode['name'] + '", '
    ret = ret[:-2] + '};\n'
    ret += 'enum Mode {'
    for mode in data['modes']:
        ret += mode['enum_name'] + ', '
    ret = ret[:-2] + '};\n'
    return ret


def generateHTMLString():
    html = open(os.path.dirname(__file__) + '/../template.html', 'r')
    lines = html.readlines()
    ret = ''
    for line in lines:
        if line.strip() == "%%%OPTIONS%%%":
            ret += generateOptionsString() + '\\\n'
        elif line.strip() == "const inputs = [];":
            ret += generateHTMLCode().replace('\n', '\\\n')
        elif len(line.strip()) > 0:
            ret += line.strip() + '\\\n'
    if update_temp:
        html = open(os.path.dirname(__file__) + '/../generated.html', 'w+')
        html.truncate(0)
        html.write(ret.replace('\\', '').replace('%i', str(len(data['modes'])-1)))
    ret = 'char HTMLTemplate[] = "' + ret[:-2].replace('"', '\\"').replace("%i", str(len(data['modes'])-1)) + '";\n'
    return ret


def generateOptionsVariable():
    ret = ""
    i = 0
    for mode in data['modes']:
        ret += "<option value='" + str(i) + "'>" + mode['name'] + "</option>"
        i = i + 1
    length = len(ret) + 1
    return "char optionsString[" + str(length) + '] = "' + ret + '";'


def generateOptionsString():
    ret = ""
    i = 0
    for mode in data['modes']:
        ret += "<option value='" + str(i) + "'>" + mode['name'] + "</option>"
        i = i + 1
    return ret


def firstLowerLetter(string):
    ret = []
    for s in string.split(' '):
        ret.append(s[0:1].lower())
    return ''.join(ret)


if __name__ == '__main__':
    data = json.load(open("config.json"))
    fileout = open(os.path.dirname(__file__) + '/../generated.h', 'w+')
    fileout.truncate(0)
    used_names = []
    for m in data['modes']:  # Give it a class name, a unique variable name.
        m['classname'] = m['name'].replace(' ', '')
        temp_name = firstLowerLetter(m['name'])
        chars = 1
        while temp_name in used_names:
            temp_name += m['name'][chars:chars + 1]
            chars += 1
        used_names.append(temp_name)
        m['v_name'] = temp_name
        m['enum_name'] = m['name'].upper().replace(' ', '_')
    fileout.write('\n'.join(data['insert']) + '\n' * 2)
    fileout.write(generateStructs())
    fileout.write(generateDataUnion())
    fileout.write('\n//**LEDOptions**\n')
    fileout.write(generateLEDOptions())
    fileout.write('\n//**SETTERS CODE**\n')
    fileout.write(generateSettersCode())
    fileout.write('\n\n\n//**HTML STRING**\n')
    fileout.write(generateHTMLString())
    fileout.close()
    print('Completed!')

# Options: Store a color, multiply RGB by brightness. Store a hue, compute a new hue for each function
