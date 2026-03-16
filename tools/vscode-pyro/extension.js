const vscode = require('vscode');
const { LanguageClient, TransportKind } = require('vscode-languageclient/node');

let client;

function activate(context) {
    const serverPath = vscode.workspace.getConfiguration('pyro').get('lspPath', 'pyro-lsp');

    const serverOptions = {
        command: serverPath,
        transport: TransportKind.stdio
    };

    const clientOptions = {
        documentSelector: [{ scheme: 'file', language: 'pyro' }]
    };

    client = new LanguageClient('pyro', 'Pyro Language Server', serverOptions, clientOptions);
    client.start();
}

function deactivate() {
    if (client) {
        return client.stop();
    }
}

module.exports = { activate, deactivate };
