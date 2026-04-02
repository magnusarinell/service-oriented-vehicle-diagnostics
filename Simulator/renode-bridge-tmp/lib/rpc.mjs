/**
 * rpc.mjs — XML-RPC engine for Renode Robot Framework server.
 *
 * Renode exposes a Robot Framework XML-RPC server on port 55555.
 * We call it via the `run_keyword` method with keyword name + args.
 *
 * Adapted from renode-visual-console/backend/lib/rpc.mjs.
 */

import http from 'node:http'
import { config } from '../config.mjs'

/** Serialise a single value as an XML-RPC <value> element. */
function xmlValue(v) {
  if (typeof v === 'number' && Number.isInteger(v)) return `<value><int>${v}</int></value>`
  return `<value><string>${String(v).replace(/&/g, '&amp;').replace(/</g, '&lt;')}</string></value>`
}

/** Build a run_keyword XML-RPC call body. */
function buildRpcCall(keyword, args = []) {
  const params = [keyword, ...args].map(xmlValue).join('\n')
  return `<?xml version="1.0"?>
<methodCall>
  <methodName>run_keyword</methodName>
  <params>
    <param>${xmlValue(keyword)}</param>
    <param><value><array><data>${args.map(xmlValue).join('')}</data></array></value></param>
  </params>
</methodCall>`
}

/** Parse the XML-RPC response into { status, output, return, error }. */
function parseRpcResponse(xml) {
  if (xml.includes('<fault>')) {
    const msg = xml.match(/<string>([\s\S]*?)<\/string>/)?.[1] ?? 'unknown fault'
    return { status: 'FAIL', error: msg }
  }
  const status = xml.match(/<member>\s*<name>status<\/name>\s*<value>\s*<string>(.*?)<\/string>/)?.[1] ?? 'FAIL'
  const output = xml.match(/<member>\s*<name>output<\/name>\s*<value>\s*<string>([\s\S]*?)<\/string>/)?.[1] ?? ''
  const ret    = xml.match(/<member>\s*<name>return<\/name>\s*<value>\s*<string>([\s\S]*?)<\/string>/)?.[1] ?? ''
  return { status, output, return: ret }
}

/** POST one XML-RPC call and return the parsed response. */
export function callXmlRpc(keyword, args = []) {
  return new Promise((resolve, reject) => {
    const body = buildRpcCall(keyword, args)
    const options = {
      hostname: config.RENODE_HOST,
      port:     config.RENODE_PORT,
      path:     '/',
      method:   'POST',
      headers:  {
        'Content-Type':   'text/xml',
        'Content-Length': Buffer.byteLength(body),
      },
    }
    const req = http.request(options, (res) => {
      let data = ''
      res.on('data', (chunk) => { data += chunk })
      res.on('end', () => resolve(parseRpcResponse(data)))
    })
    req.on('error', reject)
    req.write(body)
    req.end()
  })
}

/** Serial promise queue — prevents concurrent XML-RPC calls. */
let _queue = Promise.resolve()
export function enqueueRpc(keyword, args = []) {
  _queue = _queue.then(() => callXmlRpc(keyword, args)).catch(() => ({}))
  return _queue
}
