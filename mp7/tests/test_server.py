import aiohttp
from aiohttp import web
import chunkservice

async def test_root(aiohttp_client):
    app = web.Application()
    chunkservice.setup_app(app)
    client = await aiohttp_client(app)
    resp = await client.get('/')
    assert resp.status == 200
    assert resp.content_type == 'text/html'
    assert await resp.text() == open('index.html').read()

async def test_text(aiohttp_client):
    app = web.Application()
    chunkservice.setup_app(app)
    client = await aiohttp_client(app)
    data = aiohttp.FormData()
    data.add_field('png',
        open('tests/hiddentxt.png','rb'),
        filename='myfile.png',
        content_type='image/png')
    resp = await client.post('/extract', data=data)
    assert resp.status == 200
    assert resp.content_type.startswith('text/')
    assert await resp.text() == 'CS 340 logo\n'

async def test_png(aiohttp_client):
    app = web.Application()
    chunkservice.setup_app(app)
    client = await aiohttp_client(app)
    data = aiohttp.FormData()
    data.add_field('png',
        open('tests/hiddenpng.png','rb'),
        filename='myfile.png',
        content_type='image/png')
    resp = await client.post('/extract', data=data)
    assert resp.status == 200
    assert resp.content_type == 'image/png'
    b = await resp.read()
    assert len(b) == 639432
    assert b[100:108] == b'\x02\xea\x87\xf8)\x8f\x82\x88'
    assert b[100000:100008] == b'\xcfd\x1e\xa41P\x86\xf9'

async def test_png2(aiohttp_client):
    app = web.Application()
    chunkservice.setup_app(app)
    client = await aiohttp_client(app)
    data = aiohttp.FormData()
    data.add_field('png',
        open('tests/hiddenpng2.png','rb'),
        filename='a/longer/path.png',
        content_type='image/png')
    resp = await client.post('/extract', data=data)
    assert resp.status == 200
    assert resp.content_type == 'image/png'
    assert await resp.read() == open('tests/onered.png', 'rb').read()

async def test_gif(aiohttp_client):
    app = web.Application()
    chunkservice.setup_app(app)
    client = await aiohttp_client(app)
    data = aiohttp.FormData()
    data.add_field('png',
        open('tests/hiddengif.png','rb'),
        filename='another/long/path.png',
        content_type='image/png')
    resp = await client.post('/extract', data=data)
    assert resp.status == 200
    assert resp.content_type == 'image/gif'

async def test_nothing(aiohttp_client):
    app = web.Application()
    chunkservice.setup_app(app)
    client = await aiohttp_client(app)
    data = aiohttp.FormData()
    data.add_field('png',
        open('tests/onered.png','rb'),
        filename='myfile.png',
        content_type='image/png')
    resp = await client.post('/extract', data=data)
    assert resp.status == 422
    b = await resp.read()
    assert len(b) > 3

async def test_wrong_type(aiohttp_client):
    app = web.Application()
    chunkservice.setup_app(app)
    client = await aiohttp_client(app)
    data = aiohttp.FormData()
    data.add_field('png',
        open('tests/test_server.py','rb'),
        filename='myfile.py',
        content_type='text/x-script.phyton')
    resp = await client.post('/extract', data=data)
    assert resp.status == 415
    b = await resp.read()
    assert len(b) > 3

async def test_hiding(aiohttp_client):
    import os.path
    app = web.Application()
    chunkservice.setup_app(app)
    client = await aiohttp_client(app)
    data = aiohttp.FormData()
    data.add_field('png',
        open('tests/onered.png','rb'),
        filename='myfile.png',
        content_type='image/png')
    data.add_field('hide',
        open('Makefile','rb'),
        filename='Makefile',
        content_type='text/plain')
    resp = await client.post('/insert', data=data)
    assert resp.status == 200
    assert resp.content_type == 'image/png'
    b = await resp.read()
    assert open('Makefile','rb').read() in b
    assert len(b) == os.path.getsize('Makefile') + os.path.getsize('tests/onered.png') + 12

async def test_round_trip(aiohttp_client):
    import os.path
    app = web.Application()
    chunkservice.setup_app(app)
    client = await aiohttp_client(app)
    data = aiohttp.FormData()
    data.add_field('png',
        open('tests/onered.png','rb'),
        filename='myfile.png',
        content_type='image/png')
    data.add_field('hide',
        open('Makefile','rb'),
        filename='Makefile',
        content_type='text/plain')
    resp = await client.post('/insert', data=data)
    b = await resp.read()
    
    data2 = aiohttp.FormData()
    data2.add_field('png',
        await resp.read(),
        filename='myfile.png',
        content_type='image/png')
    resp2 = await client.post('/extract', data=data2)
    assert resp2.status == 200
    assert resp2.content_type.startswith('text/')
    b = await resp2.read()
    assert open('Makefile','rb').read() == b


