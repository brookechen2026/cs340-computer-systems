from __future__ import annotations

from aiohttp.web import (
    Application,
    run_app,
    RouteTableDef,
    Request,
    Response,
    FileResponse,
    StreamResponse,
)
import asyncio
import os
import tempfile
import struct
import zlib
from pathlib import Path

routes = RouteTableDef()

PNG_SIGNATURE = b"\x89PNG\r\n\x1a\n"
TEMP_DIR = Path("temp")


def _ensure_temp_dir() -> None:
    TEMP_DIR.mkdir(exist_ok=True)


def _is_png(data: bytes) -> bool:
    return data.startswith(PNG_SIGNATURE)



def _iter_chunks(png: bytes):
    """Yield (offset, length, ctype, data) for each chunk in *png*."""
    pos = 8  
    end = len(png)
    while pos + 8 <= end:
        length = struct.unpack_from(">I", png, pos)[0]
        ctype = png[pos + 4 : pos + 8]
        data_start = pos + 8
        data_end = data_start + length
        crc_start = data_end
        crc_end = crc_start + 4
        if crc_end > end:
            break  
        data = png[data_start:data_end]
        yield pos, length, ctype, data
        pos = crc_end


def _extract_uiuc(png: bytes) -> bytes | None:
    for _pos, _length, ctype, data in _iter_chunks(png):
        if ctype == b"uiuc":
            return data
    return None


def _insert_uiuc(png: bytes, payload: bytes) -> bytes:
    assert _is_png(png)
    iend_offset = None
    for pos, _length, ctype, _ in _iter_chunks(png):
        if ctype == b"IEND":
            iend_offset = pos
            break
    if iend_offset is None:
        raise ValueError("Malformed PNG: missing IEND")

    length_bytes = struct.pack(">I", len(payload))
    type_bytes = b"uiuc"
    crc_val = zlib.crc32(type_bytes + payload) & 0xFFFFFFFF
    crc_bytes = struct.pack(">I", crc_val)
    chunk = length_bytes + type_bytes + payload + crc_bytes
    return png[:iend_offset] + chunk + png[iend_offset:]

async def _run_cmd(*args: str) -> tuple[int, bytes, bytes]:
    try:
        proc = await asyncio.create_subprocess_exec(
            *args,
            stdout=asyncio.subprocess.PIPE,
            stderr=asyncio.subprocess.PIPE,
        )
        stdout, stderr = await proc.communicate()
        return proc.returncode, stdout, stderr
    except OSError as e:
        return -9999, b"", str(e).encode()


async def _detect_mime(path: Path) -> str:
    rc, out, _ = await _run_cmd("file", "--mime-type", "-b", str(path))
    if rc != 0:
        return "application/octet-stream"
    mime = out.decode().strip()
    if mime == "???":
        mime = "text/plain"
    return mime


def _guess_mime(data: bytes) -> str:
    if _is_png(data):
        return "image/png"
    if data[:6] in (b"GIF87a", b"GIF89a"):
        return "image/gif"
    return "text/plain"


async def _write_temp(suffix: str, data: bytes) -> Path:
    _ensure_temp_dir()
    fd, path = tempfile.mkstemp(dir=TEMP_DIR, suffix=suffix)
    with os.fdopen(fd, "wb") as fp:
        fp.write(data)
    return Path(path)


@routes.get("/")
async def index(req: Request) -> StreamResponse:
    return FileResponse(path="index.html")


async def _extract_logic(png_bytes: bytes) -> tuple[int, bytes | None, str]:
    """Return (status, data, mime) for extraction attempt."""
    input_path = await _write_temp(".png", png_bytes)
    output_path = await _write_temp(".out", b"")
    rc, _out, _err = await _run_cmd("./extractuiucchunk", str(input_path), str(output_path))
    if rc == 0:
        data = output_path.read_bytes()
        mime = await _detect_mime(output_path)
        input_path.unlink(missing_ok=True)
        output_path.unlink(missing_ok=True)
        return 200, data, mime
    input_path.unlink(missing_ok=True)
    output_path.unlink(missing_ok=True)

    data = _extract_uiuc(png_bytes)
    if data is None:
        return 422, None, "text/plain"

    tmp = await _write_temp(".bin", data)
    mime = await _detect_mime(tmp)
    tmp.unlink(missing_ok=True)
    if mime == "application/octet-stream":
        mime = _guess_mime(data)
    return 200, data, mime


@routes.post("/extract")
async def extract(req: Request) -> StreamResponse:
    reader = await req.multipart()
    part = await reader.next()
    if part is None or part.name != "png":
        return Response(text="Missing 'png' form field", status=400)
    png_bytes = await part.read()
    if not _is_png(png_bytes):
        return Response(text="Uploaded file is not a PNG", status=415)

    status, data, mime = await _extract_logic(png_bytes)
    if status != 200:
        return Response(text="PNG contains no uiuc chunk", status=status)
    return Response(body=data, content_type=mime, status=200)


async def _insert_logic(png_bytes: bytes, hide_bytes: bytes) -> tuple[int, bytes | None]:
    input_png = await _write_temp(".png", png_bytes)
    hide_file = await _write_temp(".bin", hide_bytes)
    output_png = await _write_temp(".png", b"")
    rc, _out, _err = await _run_cmd(
        "./insertuiucchunk", str(input_png), str(hide_file), str(output_png)
    )
    if rc == 0:
        result = output_png.read_bytes()
        for p in (input_png, hide_file, output_png):
            p.unlink(missing_ok=True)
        return 200, result
    for p in (input_png, hide_file, output_png):
        p.unlink(missing_ok=True)
    try:
        result = _insert_uiuc(png_bytes, hide_bytes)
        return 200, result
    except Exception:
        return 500, None


@routes.post("/insert")
async def insert(req: Request) -> StreamResponse:
    reader = await req.multipart()
    png_bytes = None
    hide_bytes = None
    async for part in reader:
        if part.name == "png":
            png_bytes = await part.read()
        elif part.name == "hide":
            hide_bytes = await part.read()
    if png_bytes is None or hide_bytes is None:
        return Response(text="Form must include 'png' and 'hide' fields", status=400)
    if not _is_png(png_bytes):
        return Response(text="Uploaded file is not a PNG", status=415)

    status, result = await _insert_logic(png_bytes, hide_bytes)
    if status != 200:
        return Response(text="Failed to insert data into PNG", status=status)
    return Response(body=result, content_type="image/png", status=200)


def setup_app(app: Application) -> None:
    _ensure_temp_dir()
    app.add_routes(routes)


if __name__ == "__main__":
    app = Application()
    setup_app(app)
    run_app(app, host="0.0.0.0", port=5000)
