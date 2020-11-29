import argparse
import logging
import os

from PIL import Image
import pywavefront
pywavefront.configure_logging(logging.CRITICAL)


def texture2header(resource, header):
    texture = Image.open(resource)
    format = texture.mode
    width, height = texture.size
    pixels = texture.tobytes()
    print('texture: {} {} {}x{}'.format(resource, format, width, height))


def model2header(resource, header):
    model = pywavefront.Wavefront(resource)
    format = ''
    vertices = []
    for _, material in model.materials.items():
        format = material.vertex_format
        for vertex in material.vertices:
            vertices.append(vertex)
    print('model: {} {} {} vertices'.format(resource, format, len(vertices)))


def shader2header(resource, header):
    _, ext = os.path.splitext(resource)
    type = 'VERT' if ext in ['.vs', '.vert'] else 'FRAG'
    with open(resource) as f:
        source = f.read()
    print('shader: {} {} {} bytes'.format(resource, type, len(source)))


def res2header(resource, header):
    _, ext = os.path.splitext(resource)
    if ext in ['.bmp', '.jpeg', '.jpg', '.png', '.tga']:
        texture2header(resource, header)
    elif ext in ['.obj']:
        model2header(resource, header)
    elif ext in ['.vs', '.fs', '.vert', '.frag']:
        shader2header(resource, header)
    else:
        raise SystemExit('Unknown resource type: {}'.format(resource))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Convert game resources into C headers')
    parser.add_argument('resource', help='input resource file')
    parser.add_argument('header', help='output header file')
    args = parser.parse_args()

    res2header(args.resource, args.header)
