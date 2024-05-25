import path from 'path';

import type {PreliminaryCompilerInfo} from '../../types/compiler.interfaces.js';
import type {ParseFiltersAndOutputOptions} from '../../types/features/filters.interfaces.js';
import {BaseCompiler} from '../base-compiler.js';

import {BaseParser} from './argument-parsers.js';

export class CompProjCompiler extends BaseCompiler {
    static get key() {
        return 'compproj';
    }

    constructor(compilerInfo: PreliminaryCompilerInfo, env) {
        super(
            {
                disabledFilters: [
                    'binary',
                    'execute',
                    'demangle',
                    'intel',
                    'labels',
                    'libraryCode',
                    'directives',
                    'commentOnly',
                    'trim',
                    'debugCalls',
                ],
                ...compilerInfo,
            },
            env,
        );
    }

    override getCompilerResultLanguageId(filters?: ParseFiltersAndOutputOptions): string | undefined {
        return 'llvm';
    }

    override getOutputFilename(dirPath: string, outputFilebase: string, key?: any): string {
        return path.join(dirPath, 'out.ll');
    }

    override optionsForBackend(backendOptions: Record<string, any>, outputFilename: string): string[] {
        return ['--output', outputFilename];
    }

    override getArgumentParser(): any {
        return BaseParser;
    }

    override optionsForFilter(
        filters: ParseFiltersAndOutputOptions,
        outputFilename: string,
        userOptions?: string[],
    ): any[] {
        return [];
    }

    override async processAsm(result, filters, options) {
        // at some point maybe a custom parser can be written, for now just don't filter anything
        return super.processAsm(
            result,
            {
                labels: false,
                binary: false,
                commentOnly: false,
                demangle: false,
                optOutput: false,
                directives: false,
                dontMaskFilenames: false,
                execute: false,
                intel: false,
                libraryCode: false,
                trim: false,
                debugCalls: false,
            },
            options,
        );
    }
}
