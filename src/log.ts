let myInfo = function (text: string) {
	console.log(text);
};

let myError = function (text: string) {
	console.log(text);
};

export function set(log) {
	myInfo = log.info;
	myError = log.error;
}

export function info(text: string) {
	myInfo(text);
}

export function error(text: string) {
	myError(text);
}
