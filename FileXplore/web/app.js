/**
 * FileXplore GUI Application
 * Vanilla JavaScript implementation with API communication
 */

class FileXploreApp {
    constructor() {
        this.currentPath = '/';
        this.selectedFiles = new Set();
        this.viewMode = 'list';
        this.sortBy = 'name';
        this.sortOrder = 'asc';
        this.commandHistory = [];
        this.commandHistoryIndex = -1;
        this.clipboard = null;
        this.clipboardOperation = null;
        this.currentPreviewPath = null;  // Track which file is being previewed
        this.navigationHistory = ['/'];  // Track navigation history
        this.navigationHistoryIndex = 0;  // Current position in history

        this.init();
    }

    async init() {
        this.setupEventListeners();
        this.setupTheme();
        await this.loadInitialData();
        this.showStatus('FileXplore GUI loaded successfully', 'success');
    }

    setupEventListeners() {
        // View controls
        document.querySelectorAll('.view-btn').forEach(btn => {
            btn.addEventListener('click', (e) => this.setViewMode(e.target.dataset.view));
        });

        // Theme toggle
        document.getElementById('theme-toggle').addEventListener('click', () => this.toggleTheme());

        // Navigation controls
        document.getElementById('nav-back-btn').addEventListener('click', () => this.navigateBack());
        document.getElementById('nav-forward-btn').addEventListener('click', () => this.navigateForward());
        document.getElementById('nav-up-btn').addEventListener('click', () => this.navigateUp());
        document.getElementById('nav-home-btn').addEventListener('click', () => this.navigateTo('/'));

        // Quick actions
        document.getElementById('create-file-btn').addEventListener('click', () => this.showCreateFileDialog());
        document.getElementById('create-folder-btn').addEventListener('click', () => this.showCreateFolderDialog());
        document.getElementById('upload-btn').addEventListener('click', () => this.showUploadDialog());

        // Toolbar
        document.getElementById('refresh-btn').addEventListener('click', () => this.refreshFileList());
        document.getElementById('search-btn').addEventListener('click', () => this.searchFiles());
        document.getElementById('search-input').addEventListener('keypress', (e) => {
            if (e.key === 'Enter') this.searchFiles();
        });
        document.getElementById('sort-select').addEventListener('change', (e) => this.setSortBy(e.target.value));
        document.getElementById('sort-order-btn').addEventListener('click', () => this.toggleSortOrder());

        // Command interface
        document.getElementById('command-input').addEventListener('keypress', (e) => {
            if (e.key === 'Enter') this.executeCommand();
        });
        document.getElementById('command-input').addEventListener('keydown', (e) => {
            if (e.key === 'ArrowUp') this.navigateCommandHistory(-1);
            if (e.key === 'ArrowDown') this.navigateCommandHistory(1);
        });
        document.getElementById('command-send-btn').addEventListener('click', () => this.executeCommand());
        document.getElementById('toggle-command').addEventListener('click', () => this.toggleCommandInterface());

        // Context menu
        document.addEventListener('contextmenu', (e) => this.handleContextMenu(e));
        document.addEventListener('click', () => this.hideContextMenu());

        // Modal close buttons
        document.getElementById('preview-close').addEventListener('click', () => this.closeModal('file-preview-modal'));
        document.getElementById('preview-save-btn').addEventListener('click', () => this.saveFilePreview());
        
        // Close modal when clicking outside
        document.getElementById('file-preview-modal').addEventListener('click', (e) => {
            if (e.target.id === 'file-preview-modal') {
                this.closeModal('file-preview-modal');
            }
        });
        
        // Close modal with ESC key, Save with Ctrl+S
        document.addEventListener('keydown', (e) => {
            const modal = document.getElementById('file-preview-modal');
            if (modal.style.display === 'flex') {
                if (e.key === 'Escape') {
                    this.closeModal('file-preview-modal');
                } else if ((e.ctrlKey || e.metaKey) && e.key === 's') {
                    e.preventDefault();  // Prevent browser save dialog
                    this.saveFilePreview();
                }
            }
        });

        // File explorer click handling
        document.getElementById('file-explorer').addEventListener('click', (e) => {
            if (e.target.closest('.file-item')) {
                this.handleFileClick(e.target.closest('.file-item'), e);
            } else if (e.target.closest('#file-explorer') && !e.target.closest('.file-item')) {
                this.clearSelection();
            }
        });

        // Drag and drop
        const fileExplorer = document.getElementById('file-explorer');
        fileExplorer.addEventListener('dragover', (e) => {
            e.preventDefault();
            e.stopPropagation();
            e.currentTarget.classList.add('drag-over');
        });

        fileExplorer.addEventListener('dragleave', (e) => {
            e.preventDefault();
            e.stopPropagation();
            if (e.target === e.currentTarget || !e.currentTarget.contains(e.relatedTarget)) {
                e.currentTarget.classList.remove('drag-over');
            }
        });

        fileExplorer.addEventListener('drop', (e) => {
            e.preventDefault();
            e.stopPropagation();
            e.currentTarget.classList.remove('drag-over');
            
            const files = Array.from(e.dataTransfer.files);
            if (files.length > 0) {
                files.forEach(file => this.uploadFile(file));
            }
        });

        // Keyboard shortcuts
        document.addEventListener('keydown', (e) => this.handleKeyboardShortcuts(e));

        // Window resize handling
        window.addEventListener('resize', () => this.handleResize());
    }

    setupTheme() {
        const savedTheme = localStorage.getItem('filexplore-theme') || 'light';
        document.documentElement.setAttribute('data-theme', savedTheme);
    }

    toggleTheme() {
        const currentTheme = document.documentElement.getAttribute('data-theme');
        const newTheme = currentTheme === 'dark' ? 'light' : 'dark';
        document.documentElement.setAttribute('data-theme', newTheme);
        localStorage.setItem('filexplore-theme', newTheme);
    }

    async loadInitialData() {
        try {
            await Promise.all([
                this.loadFileSystem(),
                this.loadSystemInfo()
            ]);
            this.updateNavigationButtons();  // Initialize navigation buttons
        } catch (error) {
            this.showStatus('Failed to load initial data', 'error');
            console.error('Error loading initial data:', error);
        }
    }

    async loadFileSystem(path = this.currentPath) {
        try {
            this.showLoading(true);
            const response = await this.apiRequest('/api/filesystem', 'GET', null, { path });

            if (response.success) {
                const data = JSON.parse(response.data);
                this.currentPath = data.currentPath;
                this.updateBreadcrumb(data.currentPath, data.parentPath);
                this.renderFileList(data.files);
                this.updateCommandPrompt();
                this.updateNavigationButtons();
            } else {
                throw new Error(response.message);
            }
        } catch (error) {
            this.showStatus('Failed to load file system', 'error');
            console.error('Error loading file system:', error);
        } finally {
            this.showLoading(false);
        }
    }

    async loadSystemInfo() {
        try {
            const response = await this.apiRequest('/api/system');

            if (response.success) {
                this.updateSystemInfo(response.data);
            }
        } catch (error) {
            console.error('Error loading system info:', error);
        }
    }

    updateSystemInfo(data) {
        const diskUsage = data.disk_usage;
        const usagePercent = ((diskUsage.used / diskUsage.total) * 100).toFixed(1);

        document.getElementById('disk-usage').textContent = `${usagePercent}% of ${this.formatFileSize(diskUsage.total)}`;
        document.getElementById('file-count').textContent = data.file_count.toLocaleString();
        document.getElementById('folder-count').textContent = data.directory_count.toLocaleString();
    }

    updateBreadcrumb(currentPath, parentPath) {
        const breadcrumb = document.getElementById('breadcrumb');
        breadcrumb.innerHTML = '';

        const parts = currentPath.split('/').filter(part => part);

        // Add root
        const rootItem = document.createElement('span');
        rootItem.className = 'breadcrumb-item';
        rootItem.textContent = '/';
        rootItem.addEventListener('click', () => this.navigateTo('/'));
        breadcrumb.appendChild(rootItem);

        // Add path parts
        parts.forEach((part, index) => {
            const separator = document.createElement('span');
            separator.className = 'breadcrumb-separator';
            separator.textContent = ' / ';
            breadcrumb.appendChild(separator);

            const item = document.createElement('span');
            item.className = 'breadcrumb-item';
            item.textContent = part;
            const path = '/' + parts.slice(0, index + 1).join('/');
            item.addEventListener('click', () => this.navigateTo(path));
            breadcrumb.appendChild(item);
        });
    }

    renderFileList(files) {
        const fileList = document.getElementById('file-list');
        const emptyState = document.getElementById('empty-state');

        if (files.length === 0) {
            fileList.style.display = 'none';
            emptyState.style.display = 'flex';
            return;
        }

        fileList.style.display = 'block';
        emptyState.style.display = 'none';

        // Sort files
        const sortedFiles = this.sortFiles(files);

        fileList.innerHTML = '';
        sortedFiles.forEach(file => {
            const fileItem = this.createFileItem(file);
            fileList.appendChild(fileItem);
        });

        this.updateSelectedCount();
    }

    createFileItem(file) {
        const item = document.createElement('div');
        item.className = 'file-item';
        item.dataset.name = file.name;
        item.dataset.type = file.type;
        item.dataset.path = this.currentPath === '/' ? `/${file.name}` : `${this.currentPath}/${file.name}`;

        const icon = document.createElement('div');
        icon.className = 'file-icon';
        icon.innerHTML = this.getFileIcon(file.type, file.name);

        const info = document.createElement('div');
        info.className = 'file-info';

        const name = document.createElement('div');
        name.className = 'file-name';
        name.textContent = file.name;

        const details = document.createElement('div');
        details.className = 'file-details';
        details.innerHTML = `
            <span>${file.type === 'directory' ? 'Directory' : this.formatFileSize(file.size)}</span>
            <span>${this.formatDate(file.modified)}</span>
        `;

        info.appendChild(name);
        info.appendChild(details);

        const actions = document.createElement('div');
        actions.className = 'file-actions';
        actions.innerHTML = `
            <button class="file-action-btn" data-action="open" title="Open">Open</button>
            <button class="file-action-btn" data-action="download" title="Download">↓</button>
            <button class="file-action-btn" data-action="delete" title="Delete">×</button>
        `;

        actions.addEventListener('click', (e) => {
            e.stopPropagation();
            this.handleFileAction(e.target.dataset.action, item.dataset.path, file);
        });

        item.appendChild(icon);
        item.appendChild(info);
        item.appendChild(actions);

        return item;
    }

    getFileIcon(type, name) {
        if (type === 'directory') {
            return '<svg width="24" height="24" viewBox="0 0 24 24" fill="currentColor"><path d="M10 4H4a2 2 0 0 0-2 2v12a2 2 0 0 0 2 2h16a2 2 0 0 0 2-2V8a2 2 0 0 0-2-2h-8l-2-2z"/></svg>';
        }

        const extension = name.split('.').pop().toLowerCase();
        const iconMap = {
            txt: '<svg width="24" height="24" viewBox="0 0 24 24" fill="currentColor"><path d="M14 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V8l-6-6z"/><path d="M14 2v6h6M16 13H8M16 17H8M10 9H8"/></svg>',
            js: '<svg width="24" height="24" viewBox="0 0 24 24" fill="currentColor"><path d="M3 3h18v18H3V3zm16 16V5H5v14h14zm-8-2h2v-6h2V9h-2V7h-2v2h-2v2h2v6z"/></svg>',
            html: '<svg width="24" height="24" viewBox="0 0 24 24" fill="currentColor"><path d="M12 17.56l4.07-1.13.55-6.1H9.38L9.2 8.3h7.6l.2-1.99H7l.56 6.01h6.89l-.23 2.58-2.22.6-2.22-.6-.14-1.66h-2l.29 3.19L12 17.56z"/><path d="M4.07 3h15.86L18.5 19.2 12 21l-6.5-1.8L4.07 3z"/></svg>',
            css: '<svg width="24" height="24" viewBox="0 0 24 24" fill="currentColor"><path d="M5 3h14a2 2 0 0 1 2 2v14a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2zm0 2v14h14V5H5zm2 2h10v2H7V7zm0 4h10v2H7v-2zm0 4h7v2H7v-2z"/></svg>',
            json: '<svg width="24" height="24" viewBox="0 0 24 24" fill="currentColor"><path d="M5 3h14a2 2 0 0 1 2 2v14a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2zm0 2v14h14V5H5zm2 2h2v10H7V7zm4 0h6v2h-6V7zm0 4h6v2h-6v-2zm0 4h4v2h-4v-2z"/></svg>',
            png: '<svg width="24" height="24" viewBox="0 0 24 24" fill="currentColor"><path d="M21 19V5a2 2 0 0 0-2-2H5a2 2 0 0 0-2 2v14a2 2 0 0 0 2 2h14a2 2 0 0 0 2-2zM8.5 13.5l2.5 3.01L14.5 12l4.5 6H5l3.5-4.5z"/></svg>',
            jpg: '<svg width="24" height="24" viewBox="0 0 24 24" fill="currentColor"><path d="M21 19V5a2 2 0 0 0-2-2H5a2 2 0 0 0-2 2v14a2 2 0 0 0 2 2h14a2 2 0 0 0 2-2zM8.5 13.5l2.5 3.01L14.5 12l4.5 6H5l3.5-4.5z"/></svg>'
        };

        return iconMap[extension] || '<svg width="24" height="24" viewBox="0 0 24 24" fill="currentColor"><path d="M14 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V8l-6-6z"/></svg>';
    }

    sortFiles(files) {
        return files.sort((a, b) => {
            // Directories first
            if (a.type !== b.type) {
                return a.type === 'directory' ? -1 : 1;
            }

            let aValue = a[this.sortBy];
            let bValue = b[this.sortBy];

            if (this.sortBy === 'name') {
                aValue = aValue.toLowerCase();
                bValue = bValue.toLowerCase();
            }

            if (this.sortBy === 'size') {
                aValue = parseInt(aValue) || 0;
                bValue = parseInt(bValue) || 0;
            }

            if (aValue < bValue) return this.sortOrder === 'asc' ? -1 : 1;
            if (aValue > bValue) return this.sortOrder === 'asc' ? 1 : -1;
            return 0;
        });
    }

    handleFileClick(fileItem, event) {
        const file = {
            name: fileItem.dataset.name,
            type: fileItem.dataset.type,
            path: fileItem.dataset.path
        };

        if (event.ctrlKey || event.metaKey) {
            this.toggleFileSelection(fileItem);
        } else if (event.shiftKey && this.selectedFiles.size > 0) {
            this.selectRange(fileItem);
        } else {
            if (file.type === 'directory') {
                this.navigateTo(file.path);
            } else {
                this.selectFile(fileItem);
                this.openFile(file.path);
            }
        }
    }

    selectFile(fileItem) {
        this.clearSelection();
        fileItem.classList.add('selected');
        this.selectedFiles.add(fileItem.dataset.path);
        this.updateSelectedCount();
    }

    toggleFileSelection(fileItem) {
        const path = fileItem.dataset.path;

        if (this.selectedFiles.has(path)) {
            fileItem.classList.remove('selected');
            this.selectedFiles.delete(path);
        } else {
            fileItem.classList.add('selected');
            this.selectedFiles.add(path);
        }

        this.updateSelectedCount();
    }

    selectRange(fileItem) {
        const allItems = Array.from(document.querySelectorAll('.file-item'));
        const currentIndex = allItems.indexOf(fileItem);
        const lastSelectedIndex = allItems.findIndex(item =>
            this.selectedFiles.has(item.dataset.path)
        );

        if (lastSelectedIndex !== -1) {
            const start = Math.min(currentIndex, lastSelectedIndex);
            const end = Math.max(currentIndex, lastSelectedIndex);

            for (let i = start; i <= end; i++) {
                const item = allItems[i];
                item.classList.add('selected');
                this.selectedFiles.add(item.dataset.path);
            }
        } else {
            this.selectFile(fileItem);
        }

        this.updateSelectedCount();
    }

    clearSelection() {
        document.querySelectorAll('.file-item.selected').forEach(item => {
            item.classList.remove('selected');
        });
        this.selectedFiles.clear();
        this.updateSelectedCount();
    }

    updateSelectedCount() {
        const count = this.selectedFiles.size;
        const text = count === 0 ? 'No items selected' :
                    count === 1 ? '1 item selected' :
                    `${count} items selected`;
        document.getElementById('selected-count').textContent = text;
    }

    async navigateTo(path) {
        if (path === this.currentPath) return;

        try {
            // Add to navigation history if it's a new path (not back/forward navigation)
            if (path !== this.navigationHistory[this.navigationHistoryIndex]) {
                // Remove any forward history if we're navigating to a new path
                this.navigationHistory = this.navigationHistory.slice(0, this.navigationHistoryIndex + 1);
                // Add new path to history
                this.navigationHistory.push(path);
                this.navigationHistoryIndex = this.navigationHistory.length - 1;
            }

            await this.loadFileSystem(path);
            this.clearSelection();
            this.updateNavigationButtons();
            // Don't show status for every navigation to avoid spam
        } catch (error) {
            this.showStatus('Failed to navigate', 'error');
        }
    }

    navigateBack() {
        if (this.navigationHistoryIndex > 0) {
            this.navigationHistoryIndex--;
            const path = this.navigationHistory[this.navigationHistoryIndex];
            this.loadFileSystem(path).then(() => {
                this.clearSelection();
                this.updateNavigationButtons();
            }).catch(() => {
                this.showStatus('Failed to navigate back', 'error');
            });
        }
    }

    navigateForward() {
        if (this.navigationHistoryIndex < this.navigationHistory.length - 1) {
            this.navigationHistoryIndex++;
            const path = this.navigationHistory[this.navigationHistoryIndex];
            this.loadFileSystem(path).then(() => {
                this.clearSelection();
                this.updateNavigationButtons();
            }).catch(() => {
                this.showStatus('Failed to navigate forward', 'error');
            });
        }
    }

    navigateUp() {
        if (this.currentPath === '/') return;
        
        const parts = this.currentPath.split('/').filter(part => part);
        if (parts.length > 0) {
            parts.pop();
            const parentPath = parts.length > 0 ? '/' + parts.join('/') : '/';
            this.navigateTo(parentPath);
        }
    }

    updateNavigationButtons() {
        const backBtn = document.getElementById('nav-back-btn');
        const forwardBtn = document.getElementById('nav-forward-btn');
        const upBtn = document.getElementById('nav-up-btn');

        // Enable/disable back button
        backBtn.disabled = this.navigationHistoryIndex <= 0;

        // Enable/disable forward button
        forwardBtn.disabled = this.navigationHistoryIndex >= this.navigationHistory.length - 1;

        // Enable/disable up button (disabled at root)
        upBtn.disabled = this.currentPath === '/';
    }

    async openFile(path) {
        try {
            const response = await this.apiRequest(`/api/file/${encodeURIComponent(path)}`);

            if (response.success) {
                this.showFilePreview(path, response.data);
            } else {
                throw new Error(response.message);
            }
        } catch (error) {
            this.showStatus('Failed to open file', 'error');
            console.error('Error opening file:', error);
        }
    }

    showFilePreview(path, content) {
        const modal = document.getElementById('file-preview-modal');
        const title = document.getElementById('preview-title');
        const contentTextarea = document.getElementById('file-preview-content');
        const saveBtn = document.getElementById('preview-save-btn');

        this.currentPreviewPath = path;  // Store the current file path
        title.textContent = `Edit: ${path.split('/').pop()}`;
        contentTextarea.value = content;

        // Make sure save button is visible
        if (saveBtn) {
            saveBtn.style.display = 'flex';
        }

        modal.style.display = 'flex';
        // Focus the textarea for immediate editing
        setTimeout(() => contentTextarea.focus(), 100);
    }

    async saveFilePreview() {
        if (!this.currentPreviewPath) {
            return;
        }

        const contentTextarea = document.getElementById('file-preview-content');
        const content = contentTextarea.value;

        try {
            // Send file content as plain text, not JSON
            const url = `/api/file/${encodeURIComponent(this.currentPreviewPath)}`;
            const response = await fetch(url, {
                method: 'POST',
                headers: {
                    'Content-Type': 'text/plain',
                },
                body: content
            });

            const result = await response.json();

            if (!response.ok) {
                throw new Error(result.message || `HTTP ${response.status}`);
            }

            if (result.success) {
                this.showStatus('File saved successfully', 'success');
            } else {
                throw new Error(result.message);
            }
        } catch (error) {
            this.showStatus('Failed to save file', 'error');
            console.error('Error saving file:', error);
        }
    }

    closeModal(modalId) {
        document.getElementById(modalId).style.display = 'none';
        this.currentPreviewPath = null;  // Clear the preview path
    }

    async executeCommand() {
        const input = document.getElementById('command-input');
        const command = input.value.trim();

        if (!command) return;

        // Add to history
        this.commandHistory.push(command);
        this.commandHistoryIndex = this.commandHistory.length;

        // Display command
        this.addCommandToHistory(command, 'command');

        try {
            const response = await this.apiRequest('/api/command', 'POST', {
                command: command.split(' ')[0],
                args: command.split(' ').slice(1)
            });

            if (response.success) {
                this.addCommandToHistory(response.message, 'output');

                // Special handling for navigation commands
                if (command.startsWith('cd ')) {
                    const path = command.substring(3).trim();
                    await this.loadFileSystem(path);
                } else if (command === 'ls' || command === 'pwd') {
                    await this.loadFileSystem();
                }
            } else {
                this.addCommandToHistory(response.message, 'error');
            }
        } catch (error) {
            this.addCommandToHistory('Command execution failed', 'error');
            console.error('Error executing command:', error);
        }

        input.value = '';
    }

    addCommandToHistory(text, type) {
        const historyDiv = document.getElementById('command-history');
        const entry = document.createElement('div');
        entry.className = `command-entry ${type}`;

        if (type === 'command') {
            entry.textContent = `$ ${text}`;
        } else {
            entry.textContent = text;
        }

        historyDiv.appendChild(entry);
        historyDiv.scrollTop = historyDiv.scrollHeight;
    }

    navigateCommandHistory(direction) {
        const input = document.getElementById('command-input');

        if (direction === -1 && this.commandHistoryIndex > 0) {
            this.commandHistoryIndex--;
            input.value = this.commandHistory[this.commandHistoryIndex];
        } else if (direction === 1 && this.commandHistoryIndex < this.commandHistory.length - 1) {
            this.commandHistoryIndex++;
            input.value = this.commandHistory[this.commandHistoryIndex];
        } else if (direction === 1 && this.commandHistoryIndex === this.commandHistory.length - 1) {
            this.commandHistoryIndex++;
            input.value = '';
        }
    }

    updateCommandPrompt() {
        const prompt = document.getElementById('command-prompt');
        prompt.textContent = `FileXplore:${this.currentPath}$ `;
    }

    toggleCommandInterface() {
        const content = document.getElementById('command-content');
        const button = document.getElementById('toggle-command');

        if (content.style.display === 'none') {
            content.style.display = 'flex';
            button.style.transform = 'rotate(180deg)';
        } else {
            content.style.display = 'none';
            button.style.transform = 'rotate(0deg)';
        }
    }

    handleFileAction(action, path, file) {
        switch (action) {
            case 'open':
                if (file.type === 'directory') {
                    this.navigateTo(path);
                } else {
                    this.openFile(path);
                }
                break;
            case 'download':
                this.downloadFile(path);
                break;
            case 'delete':
                this.deleteFile(path);
                break;
        }
    }

    async deleteFile(path) {
        if (!confirm(`Are you sure you want to delete "${path.split('/').pop()}"?`)) {
            return;
        }

        try {
            const response = await this.apiRequest('/api/command', 'POST', {
                command: 'delete',
                args: [path]
            });

            if (response.success) {
                this.showStatus('File deleted successfully', 'success');
                await this.loadFileSystem();
            } else {
                throw new Error(response.message);
            }
        } catch (error) {
            this.showStatus('Failed to delete file', 'error');
            console.error('Error deleting file:', error);
        }
    }

    downloadFile(path) {
        const link = document.createElement('a');
        link.href = `/api/file/${encodeURIComponent(path)}`;
        link.download = path.split('/').pop();
        document.body.appendChild(link);
        link.click();
        document.body.removeChild(link);
    }

    showCreateFileDialog() {
        const filename = prompt('Enter file name:');
        if (!filename) return;

        this.createFile(filename);
    }

    showCreateFolderDialog() {
        const foldername = prompt('Enter folder name:');
        if (!foldername) return;

        this.createFolder(foldername);
    }

    async createFile(name) {
        const path = this.currentPath === '/' ? `/${name}` : `${this.currentPath}/${name}`;

        try {
            const response = await this.apiRequest('/api/command', 'POST', {
                command: 'create',
                args: [path]
            });

            if (response.success) {
                this.showStatus('File created successfully', 'success');
                await this.loadFileSystem();
            } else {
                throw new Error(response.message);
            }
        } catch (error) {
            this.showStatus('Failed to create file', 'error');
            console.error('Error creating file:', error);
        }
    }

    async createFolder(name) {
        const path = this.currentPath === '/' ? `/${name}` : `${this.currentPath}/${name}`;

        try {
            const response = await this.apiRequest('/api/command', 'POST', {
                command: 'mkdir',
                args: [path]
            });

            if (response.success) {
                this.showStatus('Folder created successfully', 'success');
                await this.loadFileSystem();
            } else {
                throw new Error(response.message);
            }
        } catch (error) {
            this.showStatus('Failed to create folder', 'error');
            console.error('Error creating folder:', error);
        }
    }

    showUploadDialog() {
        const fileInput = document.getElementById('file-upload-input');
        fileInput.onchange = (e) => {
            const files = Array.from(e.target.files);
            if (files.length > 0) {
                files.forEach(file => this.uploadFile(file));
            }
            // Reset input so same file can be selected again
            fileInput.value = '';
        };
        fileInput.click();
    }

    async uploadFile(file) {
        const path = this.currentPath === '/' ? `/${file.name}` : `${this.currentPath}/${file.name}`;

        try {
            this.showStatus(`Uploading ${file.name}...`, 'info');
            
            // Read file content (handle both text and binary files)
            const content = await this.readFileAsText(file);
            
            // Send file content as plain text, not JSON
            const url = `/api/file/${encodeURIComponent(path)}`;
            const response = await fetch(url, {
                method: 'POST',
                headers: {
                    'Content-Type': 'text/plain',
                },
                body: content
            });

            const result = await response.json();

            if (!response.ok) {
                throw new Error(result.message || `HTTP ${response.status}`);
            }

            if (result.success) {
                this.showStatus(`Uploaded ${file.name} successfully`, 'success');
                await this.loadFileSystem();
            } else {
                throw new Error(result.message);
            }
        } catch (error) {
            this.showStatus(`Failed to upload ${file.name}: ${error.message}`, 'error');
            console.error('Error uploading file:', error);
        }
    }

    readFileAsText(file) {
        return new Promise((resolve, reject) => {
            const reader = new FileReader();
            reader.onload = () => resolve(reader.result);
            reader.onerror = reject;
            // Try to read as text first, fallback to binary if needed
            reader.readAsText(file);
        });
    }

    searchFiles() {
        const query = document.getElementById('search-input').value.toLowerCase();

        if (!query) {
            this.loadFileSystem();
            return;
        }

        const allItems = Array.from(document.querySelectorAll('.file-item'));

        allItems.forEach(item => {
            const name = item.dataset.name.toLowerCase();
            if (name.includes(query)) {
                item.style.display = '';
            } else {
                item.style.display = 'none';
            }
        });

        const visibleCount = allItems.filter(item => item.style.display !== 'none').length;
        this.showStatus(`Found ${visibleCount} items matching "${query}"`, 'success');
    }

    refreshFileList() {
        this.loadFileSystem();
        this.loadSystemInfo();
    }

    setViewMode(mode) {
        this.viewMode = mode;

        document.querySelectorAll('.view-btn').forEach(btn => {
            btn.classList.remove('active');
        });
        document.querySelector(`[data-view="${mode}"]`).classList.add('active');

        const fileList = document.getElementById('file-list');
        fileList.className = `file-list view-${mode}`;
    }

    setSortBy(sortBy) {
        this.sortBy = sortBy;
        this.loadFileSystem();
    }

    toggleSortOrder() {
        this.sortOrder = this.sortOrder === 'asc' ? 'desc' : 'asc';

        const button = document.getElementById('sort-order-btn');
        button.style.transform = this.sortOrder === 'asc' ? 'rotate(0deg)' : 'rotate(180deg)';

        this.loadFileSystem();
    }

    handleContextMenu(event) {
        event.preventDefault();

        const fileItem = event.target.closest('.file-item');
        if (!fileItem) return;

        this.selectFile(fileItem);

        const menu = document.getElementById('context-menu');
        menu.style.left = `${event.clientX}px`;
        menu.style.top = `${event.clientY}px`;
        menu.style.display = 'block';
    }

    hideContextMenu() {
        document.getElementById('context-menu').style.display = 'none';
    }

    handleKeyboardShortcuts(event) {
        // Prevent shortcuts when typing in inputs
        if (event.target.tagName === 'INPUT' || event.target.tagName === 'TEXTAREA') {
            return;
        }

        const key = event.key.toLowerCase();
        const ctrl = event.ctrlKey || event.metaKey;

        if (key === 'f5' || (ctrl && key === 'r')) {
            event.preventDefault();
            this.refreshFileList();
        } else if (ctrl && key === 'f') {
            event.preventDefault();
            document.getElementById('search-input').focus();
        } else if (key === 'delete') {
            event.preventDefault();
            this.deleteSelectedFiles();
        } else if (ctrl && key === 'a') {
            event.preventDefault();
            this.selectAllFiles();
        }
    }

    async deleteSelectedFiles() {
        if (this.selectedFiles.size === 0) return;

        const message = `Are you sure you want to delete ${this.selectedFiles.size} item(s)?`;
        if (!confirm(message)) return;

        try {
            for (const path of this.selectedFiles) {
                const response = await this.apiRequest('/api/command', 'POST', {
                    command: 'delete',
                    args: [path]
                });

                if (!response.success) {
                    throw new Error(response.message);
                }
            }

            this.showStatus(`${this.selectedFiles.size} item(s) deleted successfully`, 'success');
            this.clearSelection();
            await this.loadFileSystem();
        } catch (error) {
            this.showStatus('Failed to delete some items', 'error');
            console.error('Error deleting files:', error);
        }
    }

    selectAllFiles() {
        document.querySelectorAll('.file-item').forEach(item => {
            item.classList.add('selected');
            this.selectedFiles.add(item.dataset.path);
        });
        this.updateSelectedCount();
    }

    handleResize() {
        // Handle responsive layout adjustments
        if (window.innerWidth < 768) {
            // Mobile adjustments
            document.getElementById('search-input').placeholder = 'Search...';
        } else {
            document.getElementById('search-input').placeholder = 'Search files...';
        }
    }

    showLoading(show) {
        const loading = document.getElementById('loading');
        const fileList = document.getElementById('file-list');
        const emptyState = document.getElementById('empty-state');

        if (show) {
            loading.style.display = 'flex';
            fileList.style.display = 'none';
            emptyState.style.display = 'none';
        } else {
            loading.style.display = 'none';
        }
    }

    showStatus(message, type = 'info') {
        document.getElementById('status-text').textContent = message;

        // Also show toast for important messages
        if (type === 'error' || type === 'success') {
            this.showToast(message, type);
        }
    }

    showToast(message, type = 'info') {
        const container = document.getElementById('toast-container');
        const toast = document.createElement('div');
        toast.className = `toast ${type}`;

        const messageDiv = document.createElement('div');
        messageDiv.className = 'toast-message';
        messageDiv.textContent = message;

        toast.appendChild(messageDiv);
        container.appendChild(toast);

        // Auto remove after 3 seconds
        setTimeout(() => {
            toast.style.animation = 'slideOut 0.3s ease';
            setTimeout(() => {
                if (toast.parentNode) {
                    toast.parentNode.removeChild(toast);
                }
            }, 300);
        }, 3000);
    }

    async apiRequest(endpoint, method = 'GET', data = null, params = null) {
        const url = new URL(endpoint, window.location.origin);

        if (params) {
            Object.keys(params).forEach(key => {
                if (params[key] !== null && params[key] !== undefined) {
                    url.searchParams.append(key, params[key]);
                }
            });
        }

        const options = {
            method: method,
            headers: {
                'Content-Type': 'application/json',
            }
        };

        if (data && method !== 'GET') {
            options.body = JSON.stringify(data);
        }

        try {
            const response = await fetch(url.toString(), options);
            const result = await response.json();

            if (!response.ok) {
                throw new Error(result.message || `HTTP ${response.status}`);
            }

            return result;
        } catch (error) {
            console.error('API request failed:', error);
            throw error;
        }
    }

    formatFileSize(bytes) {
        if (bytes === 0) return '0 B';

        const k = 1024;
        const sizes = ['B', 'KB', 'MB', 'GB', 'TB'];
        const i = Math.floor(Math.log(bytes) / Math.log(k));

        return parseFloat((bytes / Math.pow(k, i)).toFixed(1)) + ' ' + sizes[i];
    }

    formatDate(dateString) {
        const date = new Date(dateString);
        return date.toLocaleDateString() + ' ' + date.toLocaleTimeString();
    }
}

// Initialize the application when DOM is ready
document.addEventListener('DOMContentLoaded', () => {
    window.fileXplore = new FileXploreApp();
});

// Add slide out animation
const style = document.createElement('style');
style.textContent = `
    @keyframes slideOut {
        to {
            transform: translateX(100%);
            opacity: 0;
        }
    }
`;
document.head.appendChild(style);