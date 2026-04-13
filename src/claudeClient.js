/**
 * ClawCity - Claude Client via OpenRouter
 * Uses OpenRouter API (free tier) with OpenAI-compatible SDK
 */
import 'dotenv/config';
import OpenAI from 'openai';

const OPENROUTER_API_KEY = process.env.OPENROUTER_API_KEY;

if (!OPENROUTER_API_KEY) {
  throw new Error('OPENROUTER_API_KEY not found (env var). Put it in .env as OPENROUTER_API_KEY=... or export it before running.');
}

export function createClient() {
  return new OpenAI({
    apiKey: OPENROUTER_API_KEY,
    baseURL: 'https://openrouter.ai/api/v1',
    defaultHeaders: {
      'HTTP-Referer': 'https://github.com/diego31-10/clawcity',
      'X-Title': 'ClawCity',
    },
  });
}

function sleep(ms) {
  return new Promise(r => setTimeout(r, ms));
}

function unique(arr) {
  return [...new Set(arr.filter(Boolean))];
}

export async function askClaude(
  client,
  systemPrompt,
  userMessage,
  model = (process.env.CLAUDE_MODEL || 'openrouter/free')
) {
  const fallbacks = (process.env.MODEL_FALLBACKS || '')
    .split(',')
    .map(s => s.trim())
    .filter(Boolean);

  const modelsToTry = unique([model, ...fallbacks]);

  const maxRetriesPerModel = 3;

  let lastErr;
  for (const m of modelsToTry) {
    for (let attempt = 0; attempt <= maxRetriesPerModel; attempt++) {
      try {
        const response = await client.chat.completions.create({
          model: m,
          messages: [
            { role: 'system', content: systemPrompt },
            { role: 'user', content: userMessage },
          ],
          max_tokens: 768,
        });
        return response.choices[0].message.content;
      } catch (err) {
        lastErr = err;
        const status = err?.status || err?.response?.status;
        const msg = err?.message || '';

        // 429 = rate limit / overloaded (muy común en modelos free)
        // 503 = overloaded
        const retryable = status === 429 || status === 503 || /rate limit|overloaded|try again/i.test(msg);

        if (!retryable || attempt === maxRetriesPerModel) break;

        // Exponential backoff with jitter
        const base = 600 * Math.pow(2, attempt);
        const jitter = Math.floor(Math.random() * 250);
        await sleep(base + jitter);
      }
    }
  }

  throw lastErr;
}
