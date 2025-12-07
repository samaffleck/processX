// Flowsheet storage library (using in-memory storage for development)
// For production, replace with Vercel KV, Prisma, or another database

export interface FlowsheetMetadata {
  id: string;
  name: string;
  description?: string;
  organizationId: string;
  createdBy: string; // Clerk user ID
  createdByName: string; // User display name
  createdAt: Date;
  updatedAt: Date;
  updatedBy: string; // Clerk user ID
  updatedByName: string; // User display name
  version: number;
  data: object; // The actual flowsheet JSON
}

export interface FlowsheetVersion {
  version: number;
  data: object;
  updatedBy: string;
  updatedByName: string;
  updatedAt: Date;
  changeDescription?: string;
}

// In-memory storage (replace with database in production)
const flowsheets = new Map<string, FlowsheetMetadata>();
const flowsheetVersions = new Map<string, FlowsheetVersion[]>(); // key: flowsheetId

export async function createFlowsheet(
  data: {
    name: string;
    description?: string;
    organizationId: string;
    createdBy: string;
    createdByName: string;
    data: object;
  }
): Promise<FlowsheetMetadata> {
  const id = `flowsheet_${Date.now()}_${Math.random().toString(36).substr(2, 9)}`;
  const now = new Date();

  const flowsheet: FlowsheetMetadata = {
    id,
    name: data.name,
    description: data.description,
    organizationId: data.organizationId,
    createdBy: data.createdBy,
    createdByName: data.createdByName,
    createdAt: now,
    updatedAt: now,
    updatedBy: data.createdBy,
    updatedByName: data.createdByName,
    version: 1,
    data: data.data,
  };

  flowsheets.set(id, flowsheet);

  // Initialize version history
  flowsheetVersions.set(id, [{
    version: 1,
    data: data.data,
    updatedBy: data.createdBy,
    updatedByName: data.createdByName,
    updatedAt: now,
    changeDescription: 'Initial version',
  }]);

  return flowsheet;
}

export async function getFlowsheetsByOrganization(
  organizationId: string
): Promise<FlowsheetMetadata[]> {
  const results: FlowsheetMetadata[] = [];

  for (const flowsheet of flowsheets.values()) {
    if (flowsheet.organizationId === organizationId) {
      results.push(flowsheet);
    }
  }

  // Sort by updated date (newest first)
  return results.sort((a, b) => b.updatedAt.getTime() - a.updatedAt.getTime());
}

export async function getFlowsheetById(
  id: string,
  organizationId: string
): Promise<FlowsheetMetadata | null> {
  const flowsheet = flowsheets.get(id);

  if (!flowsheet || flowsheet.organizationId !== organizationId) {
    return null;
  }

  return flowsheet;
}

export async function updateFlowsheet(
  id: string,
  organizationId: string,
  data: {
    name?: string;
    description?: string;
    data?: object;
    updatedBy: string;
    updatedByName: string;
    changeDescription?: string;
  }
): Promise<FlowsheetMetadata | null> {
  const flowsheet = flowsheets.get(id);

  if (!flowsheet || flowsheet.organizationId !== organizationId) {
    return null;
  }

  const now = new Date();

  // Update metadata
  if (data.name !== undefined) flowsheet.name = data.name;
  if (data.description !== undefined) flowsheet.description = data.description;
  if (data.data !== undefined) {
    flowsheet.data = data.data;
    flowsheet.version += 1;

    // Add to version history
    const versions = flowsheetVersions.get(id) || [];
    versions.push({
      version: flowsheet.version,
      data: data.data,
      updatedBy: data.updatedBy,
      updatedByName: data.updatedByName,
      updatedAt: now,
      changeDescription: data.changeDescription,
    });
    flowsheetVersions.set(id, versions);
  }

  flowsheet.updatedAt = now;
  flowsheet.updatedBy = data.updatedBy;
  flowsheet.updatedByName = data.updatedByName;

  flowsheets.set(id, flowsheet);

  return flowsheet;
}

export async function deleteFlowsheet(
  id: string,
  organizationId: string
): Promise<boolean> {
  const flowsheet = flowsheets.get(id);

  if (!flowsheet || flowsheet.organizationId !== organizationId) {
    return false;
  }

  flowsheets.delete(id);
  flowsheetVersions.delete(id);

  return true;
}

export async function getFlowsheetVersionHistory(
  id: string,
  organizationId: string
): Promise<FlowsheetVersion[]> {
  const flowsheet = flowsheets.get(id);

  if (!flowsheet || flowsheet.organizationId !== organizationId) {
    return [];
  }

  return flowsheetVersions.get(id) || [];
}

export async function restoreFlowsheetVersion(
  id: string,
  organizationId: string,
  version: number,
  updatedBy: string,
  updatedByName: string
): Promise<FlowsheetMetadata | null> {
  const flowsheet = flowsheets.get(id);

  if (!flowsheet || flowsheet.organizationId !== organizationId) {
    return null;
  }

  const versions = flowsheetVersions.get(id) || [];
  const targetVersion = versions.find(v => v.version === version);

  if (!targetVersion) {
    return null;
  }

  // Create a new version with the restored data
  return updateFlowsheet(id, organizationId, {
    data: targetVersion.data,
    updatedBy,
    updatedByName,
    changeDescription: `Restored from version ${version}`,
  });
}
