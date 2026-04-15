/**
 * AgentHQ - Claude Client via GitHub Models
 * Uses GitHub Models - FREE with GitHub token
 */
import 'dotenv/config';
import OpenAI from 'openai';

const GITHUB_TOKEN = process.env.GITHUB_TOKEN;

if (!GITHUB_TOKEN) {
  throw new Error('GITHUB_TOKEN not found (env var). Create one at https://github.com/settings/tokens (needs repo scope)');
}

export function createClient() {
  return new OpenAI({
    baseURL: 'https://models.github.ai/inference',
    apiKey: GITHUB_TOKEN,
  });
}

function sleep(ms) {
  return new Promise(r => setTimeout(r, ms));
}

export async function askClaude(
  client,
  systemPrompt,
  userMessage,
  model = (process.env.CLAUDE_MODEL || 'openai/gpt-4o')
) {
  const maxRetries = 3;
  let lastErr;

  for (let attempt = 0; attempt <= maxRetries; attempt++) {
    try {
      const response = await client.chat.completions.create({
        model: model,
        messages: [
          { role: 'system', content: systemPrompt },
          { role: 'user', content: userMessage },
        ],
        max_tokens: 500,
      });

      return response.choices[0].message.content;
    } catch (err) {
      lastErr = err;
      const msg = err?.message || '';
      const status = err?.status;

      console.error(`[Model: ${model}] Status: ${status}, Error: ${msg.substring(0, 100)}`);

      const retryable = /timeout|429|503|rate/i.test(msg);
      if (!retryable || attempt === maxRetries) break;

      // Exponential backoff
      const base = 600 * Math.pow(2, attempt);
      const jitter = Math.floor(Math.random() * 250);
      await sleep(base + jitter);
    }
  }

  throw lastErr;
}
