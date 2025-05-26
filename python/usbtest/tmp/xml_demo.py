import xml.etree.cElementTree as ET
import copy

def traverse(element, level=0):
    print('  ' * level + f"{element.tag} {element.attrib}")
    for child in element:
        traverse(child, level + 1)

def parse_xml_file(xml_file: str):
    tree = ET.parse(xml_file)
    root = tree.getroot()
    # traverse(root)
    neibs = root.findall('.//ExternalHub')
    dest_obj_attr = None
    dest_obj_parent_attr = None
    for neib in neibs:
        for sub_neib in neib:
            name = sub_neib.get('DeviceId')
            # print(f'name: {name}')
            if (name is not None) and ('1A86' in name):
                print('found')
                dest_obj_attr = copy.deepcopy(sub_neib.attrib)
                dest_obj_parent_attr = copy.deepcopy(neib.attrib)
                break
    print(f'desk_neib: {dest_obj_attr}')
    print(f'dest_obj_parent: {dest_obj_parent_attr}')


if __name__ == "__main__":
    print("begin")
    xml_file='test.xml'
    parse_xml_file(xml_file)


    print("end")