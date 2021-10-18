import json
import os

from data import *

debug_setters = False
update_temp = True


def generateDataUnion():
    ret = "union Data {\n"

    for mode in data['modes']:
        if not mode.get('um') and not mode.get('im'):
            ret += ""
        else:
            ret += '     ' + mode['classname'] + "Data " + mode['v_name'] + ';\n'
    ret += '} d;\n\n'
    return ret


def generateStructs():
    ret = ""
    for mode in data['modes']:
        if mode.get('um') or mode.get('im'):
            ret += 'struct ' + mode['classname'] + 'Data {\n'
            if mode.get('um'):
                for member in mode.get('um'):
                    ret += getStructVariableName(member) + ";\n"
            if mode.get('im'):
                for member in mode.get('im'):
                    ret += getStructVariableName(member) + ";\n"
            ret += '};\n\n'
    return ret


def generateHTMLCode():
    vla_code = []
    ret = 'const inputs = {\n'
    for mode in data['modes']:
        onselect = []
        ret += s(4) + '\'' + mode['name'] + '\': [\''
        if mode.get('um'):
            for i, member in enumerate(mode.get('um')):
                vla = member[1].get('VLA')
                if not vla:
                    ret += f'<div>{member[1]["visible_name"]}: <input type="{member[1]["html_type"]}" id="{i}"'
                    # If it is a range, it might have a max. Set it.
                    try:
                        if member[1]["html_type"] == 'range':
                            if mode.get('max') and mode.get('max').get(str(i)):
                                ret += ' max="' + str(mode.get('max').get(str(i))) + '"'
                            else:
                                ret += ' max="255"'
                    except (KeyError, IndexError):
                        printl(
                            "Fail: Member \"" + member[1]["visible_name"] + "\" has no default max in mode \"" + mode[
                                'name'] +
                            "\". Add one to continue.")
                        exit(-1)
                    # it might have a default value
                    try:
                        if member[1]["html_type"] == 'color' or member[1]["html_type"] == 'range':
                            ret += ' value="' + str(mode['default'][i]) + '"'
                        if member[1]["html_type"] == 'checkbox':
                            if mode['default'][i]:
                                ret += ' checked'
                    except (KeyError, IndexError):
                        printl(
                            "Fail: Member \"" + member[1]["visible_name"] + "\" has no default parameter in mode \"" +
                            mode[
                                'name'] + "\". Add one to continue.")
                        exit(-1)
                    oninput = ["u();"]
                    if member[1].get('VLA4'):
                        vlaID = member[1].get('VLA4')
                        oninput.insert(0, f"cVLA({i},{vlaID});")
                        onselect.append(f"cVLA({i},{vlaID});")
                    if member[1].get('if4'):
                        ifIDs = member[1].get('if4')
                        oninput.insert(0, f"cIF({i},[{','.join(ifIDs)}]);")
                        onselect.append(f"cIF({i},[{','.join(ifIDs)}]);")

                    ret += f' oninput="{"".join(oninput)}"></div>'
                else:
                    # We have a VLA. The contents of the vla will be generated when the slider moves. The slider will
                    ret += '<div id=\"' + str(i) + '\"></div>'
                    default_element = member[1]["html_type"]
                    vla_code.append(f'vlas[\'{i}\'] = \'{default_element}\';')
        ons = ''
        if len(onselect) > 0:
            ons = f',()=>{{{"".join(onselect)}}}'.replace("\\'", "'")
        ret += '\',' + str(len(mode.get('um') if mode.get('um') else [])) + f'{ons}],\n'
    ret += '};\n'
    for line in vla_code:
        ret += line
    return ret + '\n'


# This function will generate the code required to take the encoded parameters in order and set variables.
# For example, for dot, a color should be send back and set equal to the variable.
def generateSettersCode():
    ret = 'void fillInArgs(Mode selected, ESP8266WebServer &server) {\n'
    ret += s(1) + 'char buff[7];\n'
    ret += s(1) + 'char nameBuff[4];\n'
    ret += s(1) + 'switch (selected) {\n'
    for mode in data['modes']:
        if mode.get('um'):
            ret += s(1) + 'case ' + mode['enum_name'] + ':\n'
            i = 0
            for member in mode.get('um'):
                vla = member[1].get("VLA")
                if vla is not None:
                    vla_len_name = getName(mode, vla['index'])
                    ret += s(2) + 'delete[] ' + member[1]['member_name'] + ';\n'
                    ret += s(2) + member[1]['member_name'] + ' = new ' + member[1][
                        'type'] + '[' + vla_len_name + ']();\n'
                    ret += getForLoop(vla_len_name)
                    ret += s(3) + f'sprintf(nameBuff, "{i + 1}s%d", i);\n'
                    ret += getSetter(member[1]['html_type'], member[1]["member_name"] + '[i]', 'nameBuff',
                                     3) + '        }\n'
                    printl('Reminder: Add "delete[] ' + member[1]['member_name'] + ';" to the end code of ' + mode[
                        'name'] + ' mode.')
                    printl('Reminder: Add "' + member[1]['member_name'] + ' = new ' + member[1]['type'] + '[' + str(
                        vla['default']) + ']();" to the start of ' + mode[
                               'name'] + ' mode.')
                    if debug_setters:
                        ret += getForLoop(vla_len_name)
                        ret += s(3) + 'Serial.print("' + member[1]['member_name'] + ': ");\n' + s(
                            3) + 'Serial.println(' + member[1]['member_name'] + '[i]);\n'
                        ret += s(2) + '}\n'
                else:
                    ret += getSetter(member[1]['html_type'], member[1]['member_name'], f'"p{i}"')
                    if debug_setters:
                        ret += s(
                            2) + f'Serial.print("{member[1]["member_name"]}: ");\n{s(2)}Serial.println({member[1]["member_name"]});\n'
                i = i + 1
            ret += s(2) + 'break;\n'
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
            ret += generateHTMLCode().replace('\n', '\\n\\\n')
        elif len(line.strip()) > 0:
            ret += line.strip() + '\\n\\\n'
    if update_temp:
        html = open(os.path.dirname(__file__) + '/../generated.html', 'w+')
        html.truncate(0)
        html.write(ret.replace('\\n\\', ''))
    ret = 'char HTMLTemplate[] = "' + ret[:-2].replace('"', '\\"').replace("\\'", "\\\\\\'") + '";\n'
    return ret


def generateOptionsString():
    ret = ""
    i = 0
    for mode in data['modes']:
        ret += "<option value='" + str(i) + "'>" + mode['name'] + "</option>"
        i = i + 1
    return ret


def generateStringifyParams():
    maxSize = 0
    ret = "void stringifyParams(Mode selected) {\n    switch (selected) {\n"
    for mode in data['modes']:
        thisSize = 0
        ret += s(1) + 'case ' + mode['enum_name'] + ':\n'
        arguments = ['selected']
        ret += s(2) + 'sprintf(spBuffer, "%i|'
        if mode.get('um'):
            for member in mode.get('um'):
                vla = member[1].get('VLA')
                if vla is None:
                    thisSize += 1 + member[1]['html_input_string_size']
                    ret += member[1]['html_input_format'] + '|'
                    if member[1]["html_type"] == "range" or member[1]["html_type"] == 'checkbox':
                        arguments.append(member[1]['member_name'])
                    if member[1]["html_type"] == "color":
                        # First, we have to generate a char array with the 6 character html string.
                        arguments.append(member[1]['member_name']+".r")
                        arguments.append(member[1]['member_name']+".g")
                        arguments.append(member[1]['member_name']+".b")
                else:
                    ret += '|'
        ret = ret[:-1] + '", ' + ', '.join(arguments) + ');\n'
        ret += s(2) + 'break;\n'
        maxSize = max(thisSize, maxSize)
    ret += '    }\n}\n\n'
    return f'char spBuffer[{maxSize+1}];\n{ret}'


if __name__ == '__main__':
    data = json.load(open("config.json"))
    fileout = open(os.path.dirname(__file__) + '/../generated.h', 'w+')
    fileout.truncate(0)
    used_names = []
    for m in data['modes']:  # Give it a class name, a unique variable name. Also create the VLA variables if needed.
        m['classname'] = m['name'].replace(' ', '')
        temp_name = firstLowerLetter(m['name'])
        chars = 1
        while temp_name in used_names:
            temp_name += m['name'][chars:chars + 1]
            chars += 1
        used_names.append(temp_name)
        m['v_name'] = temp_name
        m['enum_name'] = m['name'].upper().replace(' ', '_')
        fileout.write(f'//{m["name"]} -> {m["v_name"]}\n')
    # Look for and create VLA length sliders/bytes.
    for m in data['modes']:
        if m.get('um'):
            for index, me in enumerate(m.get('um')):
                if type(me) == list and me[1].get('VLA'):
                    if me[1].get('VLA').get('index') == "create":
                        # Insert this byte before the array.
                        max_len = me[1].get('VLA').get('max')
                        default_len = me[1].get('VLA').get('default')
                        if me[1].get('if'):
                            m.get('um').insert(index,
                                               ["byte " + (" ".join(me[0].split(' ')[1:]) + " Length").replace('*', ''),
                                                {'if': me[1].get('if')}])
                        else:
                            m.get('um').insert(index,
                                               ["byte " + (" ".join(me[0].split(' ')[1:]) + " Length").replace('*', ''), {}])
                        m['um'][index][1]['VLA4'] = index + 1
                        if m.get('max'):
                            m.get('max')[str(index)] = max_len
                        if m.get('default'):
                            m.get('default').insert(index, default_len)
                        me[1]['VLA']['index'] = index
                elif type(me) != list:
                    me = m['um'][index] = [me, {}]
    # Look for and create IF dependencies.
    for m in data['modes']:
        if m.get('um'):
            for index, me in enumerate(m.get('um')):
                if me[1].get('if'):
                    # Index refers to the the thing that needs to be switched.
                    r = ''
                    ifd = me[1]['if']
                    if ifd[0] == '!':
                        r += '!'
                        ifd = ifd[1:]
                    else:
                        r += 'N'
                    if not m['um'][int(ifd)][1].get('if4'):
                        m['um'][int(ifd)][1]['if4'] = []
                    m['um'][int(ifd)][1]['if4'].append(f'\\\'{r}{index}\\\'')
    for m in data['modes']:
        if m.get('um'):
            for index, me in enumerate(m.get('um')):
                me[1]['type'] = me[0].split(' ')[0]
                me[1]['html_type'], me[1]['html_input_format'], me[1]['html_input_string_size'] = {
                    "CRGB": ("color", '#%02X%02X%02X', 12),
                    "byte": ("range", '%i', 3),
                    "bool": ("checkbox", '%i', 1),
                    "boolean": ("checkbox", '%i', 1)
                }[me[1]['type']]
                me[1]['variable_name'] = convertToCamelCase(' '.join(me[0].split(' ')[1:])).replace('*', '')
                me[1]['member_name'] = 'd.' + m['v_name'] + '.' + me[1]['variable_name']
                me[1]['visible_name'] = ' '.join(me[0].split(' ')[1:]).replace('*', '')

    fileout.write('\n'.join(data['insert']) + '\n' * 2)
    fileout.write(generateStructs())
    fileout.write(generateDataUnion())
    fileout.write('\n//**LEDOptions**\n')
    fileout.write(generateLEDOptions())
    fileout.write('\n//**SETTERS CODE**\n')
    fileout.write(generateSettersCode())
    fileout.write('\n//**HTML STRING**\n')
    fileout.write(generateHTMLString())
    fileout.write('\n//**STRINGIFY PARAMS**\n')
    fileout.write(generateStringifyParams())
    fileout.close()
    print('Completed!')

    # Options: Store a color, multiply RGB by brightness. Store a hue, compute a new hue for each function
