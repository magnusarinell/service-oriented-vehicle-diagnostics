import { ref, readonly } from 'vue'
import type {
  SovdCapability,
  SovdDataItem,
  SovdFault,
  SovdOperation,
  SovdOperationRequest,
  SovdOperationResult,
} from '@/types/sovd'

const BASE = '/api/v1/ecu'

async function apiFetch<T>(url: string, init?: RequestInit): Promise<T> {
  const res = await fetch(url, init)
  if (!res.ok) {
    const text = await res.text().catch(() => res.statusText)
    throw new Error(`${res.status} ${text}`)
  }
  if (res.status === 204) return undefined as T
  return res.json() as Promise<T>
}

export function useEcu(ecuId: string) {
  const loading = ref(false)
  const error = ref<string | null>(null)

  async function run<T>(fn: () => Promise<T>): Promise<T | null> {
    loading.value = true
    error.value = null
    try {
      return await fn()
    } catch (e) {
      error.value = e instanceof Error ? e.message : String(e)
      return null
    } finally {
      loading.value = false
    }
  }

  function fetchCapabilities() {
    return run(() => apiFetch<SovdCapability[]>(`${BASE}/${ecuId}/capabilities`))
  }

  function fetchData() {
    return run(() => apiFetch<SovdDataItem[]>(`${BASE}/${ecuId}/data`))
  }

  function fetchDataItem(dataId: string) {
    return run(() => apiFetch<SovdDataItem>(`${BASE}/${ecuId}/data/${dataId}`))
  }

  function fetchFaults() {
    return run(() => apiFetch<SovdFault[]>(`${BASE}/${ecuId}/faults`))
  }

  function clearFaults() {
    return run(() =>
      apiFetch<void>(`${BASE}/${ecuId}/faults`, { method: 'DELETE' }),
    )
  }

  function fetchOperations() {
    return run(() => apiFetch<SovdOperation[]>(`${BASE}/${ecuId}/operations`))
  }

  function executeOperation(opId: string, params?: SovdOperationRequest) {
    return run(() =>
      apiFetch<SovdOperationResult>(`${BASE}/${ecuId}/operations/${opId}/execute`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(params ?? {}),
      }),
    )
  }

  return {
    loading: readonly(loading),
    error: readonly(error),
    fetchCapabilities,
    fetchData,
    fetchDataItem,
    fetchFaults,
    clearFaults,
    fetchOperations,
    executeOperation,
  }
}
