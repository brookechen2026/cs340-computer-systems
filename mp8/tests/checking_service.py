"""
This file contains code to contact a weather forecast checking service,
written and hosted by course staff
"""

import aiohttp
import json

async def check(course, forecast):
  async with aiohttp.ClientSession() as session:
    try:
      async with session.post('http://sp25-cs340-adm.cs.illinois.edu:3408/check/'+course, data=json.dumps(forecast).encode('utf-8')) as resp:
        return await resp.json()
    except:
      return 'Checker not running'

