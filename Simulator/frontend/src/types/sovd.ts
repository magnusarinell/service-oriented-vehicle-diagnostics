// Mirror of C# SovdModels.cs

export interface SovdCapability {
  name: string
  description: string
  category: string
}

export interface SovdDataItem {
  id: string
  name: string
  value: string
  unit: string
}

export interface SovdFault {
  code: string
  description: string
  severity: 'high' | 'medium' | 'low'
  status: 'active' | 'pending' | 'inactive'
}

export interface SovdOperation {
  id: string
  name: string
  description: string
}

export interface SovdOperationRequest {
  parameters?: Record<string, string>
}

export interface SovdOperationResult {
  status: 'ok' | 'error'
  message?: string
  outputParameters?: Record<string, string>
}
