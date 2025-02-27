import {BaseTool} from './base-tool.js';

export class LliTool extends BaseTool {
    static get key() {
        return 'lli-tool';
    }

    override async runTool(compilationInfo: Record<any, any>, inputFilepath?: string, args?: string[]) {
        if (compilationInfo.filters.binary || compilationInfo.filters.binaryObject) {
            return this.createErrorResponse(`${this.tool.name ?? 'lli'} requires a text file with LLVM IR`);
        }

        return super.runTool(compilationInfo, compilationInfo.outputFilename, args);
    }
}
