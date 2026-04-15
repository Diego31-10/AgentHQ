/**
 * AgentHQ - State Manager
 * Tracks real-time state of each agent with metadata
 */

let taskMetadata = { format: null, topic: null };
let agentDescriptions = { xocas: '', momo: '', llados: '' };

const states = {
  xocas:  { state: 'IDLE', task: 'Waiting...' },
  momo:   { state: 'IDLE', task: 'Waiting...' },
  llados: { state: 'IDLE', task: 'Waiting...' },
};

export const STATE_COLORS = {
  IDLE: 'yellow',
  WORKING: 'green',
  COMMUNICATING: 'blue',
};

export const STATE_EMOJI = {
  IDLE: '🟡',
  WORKING: '🟢',
  COMMUNICATING: '🔵',
};

export function getStates() {
  return JSON.parse(JSON.stringify({
    ...states,
    metadata: taskMetadata,
    descriptions: agentDescriptions
  }));
}

export function setState(agent, state, task) {
  states[agent] = { state, task };
}

export function setMetadata(format, topic) {
  taskMetadata = { format, topic };
}

export function setAgentDescription(agent, description) {
  agentDescriptions[agent] = description;
}

export function resetAll() {
  for (const agent of Object.keys(states)) {
    states[agent] = { state: 'IDLE', task: 'Waiting...' };
  }
  taskMetadata = { format: null, topic: null };
  // NO resetear agentDescriptions - se generan una vez y se usan toda la ejecución
}
