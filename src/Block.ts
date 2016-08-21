import * as fs from 'fs';

export class Block {
	out: number;
	indentation: number;

	constructor(out: number, indentation: number) {
		this.out = out;
		this.indentation = indentation;
	}

	indent() {
		++this.indentation;
	}

	unindent() {
		--this.indentation;
	}

	tag(name: string, value: string) {
		this.p("<" + name + ">" + value + "</" + name + ">");
	}

	tagStart(name: string) {
		this.p("<" + name + ">");
		this.indent();
	}

	tagEnd(name: string) {
		this.unindent();
		this.p("</" + name + ">");
	}

	p(line: string) {
		if (line === undefined) line = '';
		let tabs = '';
		for (let i = 0; i < this.indentation; ++i) tabs += '\t';
		let data = new Buffer(tabs + line + '\n');
		fs.writeSync(this.out, data, 0, data.length, null);
	}
}
