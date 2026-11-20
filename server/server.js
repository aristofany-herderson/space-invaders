import http from 'node:http';
import { readFile } from 'node:fs/promises';
import pg from 'pg';

const PORT = process.env.PORT || 3000;
const pool = new pg.Pool({ connectionString: process.env.DATABASE_URL });
const clients = new Set(); // respostas SSE abertas
// ponytail: aviso em memoria, so funciona com 1 instancia; com varias, trocar por LISTEN/NOTIFY

await pool.query(await readFile(new URL('schema.sql', import.meta.url), 'utf8'));
const page = await readFile(new URL('public/ranking.html', import.meta.url));

async function top(limit = 10) {
  const { rows } = await pool.query(
    'SELECT name, score, created_at FROM scores ORDER BY score DESC, created_at LIMIT $1',
    [limit]
  );
  return rows;
}

function json(res, status, body) {
  res.writeHead(status, { 'Content-Type': 'application/json; charset=utf-8' });
  res.end(JSON.stringify(body));
}

async function readJson(req) {
  let data = '';
  for await (const chunk of req) {
    data += chunk;
    if (data.length > 1024) throw new SyntaxError('body muito grande');
  }
  return JSON.parse(data);
}

async function broadcast() {
  const msg = `data: ${JSON.stringify(await top())}\n\n`;
  for (const res of clients) res.write(msg);
}

async function postScore(req, res) {
  let body;
  try {
    body = await readJson(req);
  } catch {
    return json(res, 400, { error: 'JSON invalido' });
  }
  const name = typeof body?.name === 'string' ? body.name.trim() : '';
  const { score } = body ?? {};
  if (name.length < 1 || name.length > 16)
    return json(res, 400, { error: 'name deve ter de 1 a 16 caracteres' });
  if (!Number.isInteger(score) || score < 0 || score > 2147483647)
    return json(res, 400, { error: 'score deve ser um inteiro >= 0' });

  const { rows } = await pool.query(
    'INSERT INTO scores (name, score) VALUES ($1, $2) RETURNING id, name, score, created_at',
    [name, score]
  );
  json(res, 201, rows[0]);
  broadcast().catch(err => console.error('broadcast:', err));
}

async function stream(req, res) {
  res.writeHead(200, {
    'Content-Type': 'text/event-stream',
    'Cache-Control': 'no-cache',
    Connection: 'keep-alive',
  });
  res.write(`data: ${JSON.stringify(await top())}\n\n`);
  clients.add(res);
  const ping = setInterval(() => res.write(': ping\n\n'), 25000); // mantem proxies sem derrubar
  req.on('close', () => {
    clearInterval(ping);
    clients.delete(res);
  });
}

const server = http.createServer(async (req, res) => {
  const url = new URL(req.url, 'http://localhost');
  try {
    if (req.method === 'GET' && url.pathname === '/') {
      res.writeHead(200, { 'Content-Type': 'text/html; charset=utf-8' });
      return res.end(page);
    }
    if (req.method === 'GET' && url.pathname === '/scores') {
      const limit = Math.min(Math.max(parseInt(url.searchParams.get('limit')) || 10, 1), 100);
      return json(res, 200, await top(limit));
    }
    if (req.method === 'POST' && url.pathname === '/scores') return await postScore(req, res);
    if (req.method === 'GET' && url.pathname === '/scores/stream') return await stream(req, res);
    json(res, 404, { error: 'rota nao encontrada' });
  } catch (err) {
    console.error(err);
    if (!res.headersSent) json(res, 500, { error: 'erro interno' });
    else res.end();
  }
});

server.listen(PORT, () => console.log(`ranking em http://localhost:${PORT}`));
