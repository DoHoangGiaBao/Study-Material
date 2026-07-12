import tkinter as tk
from tkinter import ttk, messagebox
import collections

# --- Process class ---
class Process:
    def __init__(self, pid, arrival, burst, priority):
        self.pid = pid
        self.arrival = arrival
        self.burst = burst
        self.priority = priority
        self.remaining_time = burst
        self.completion_time = 0
        self.waiting_time = 0
        self.turnaround_time = 0

# --- GUI class ---
class SchedulerGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("CPU Scheduling Simulator")
        self.root.geometry("1000x800")

        # 1. Input table section
        self.setup_input_table()
        
        # 2. Control section
        self.setup_controls()
        
        # 3. Gantt Chart Area (with Scrollbar)
        tk.Label(self.root, text="Gantt Chart", font=('Arial', 12, 'bold')).pack(pady=5)
        self.gantt_frame = tk.Frame(self.root)
        self.gantt_frame.pack(fill="x", padx=20)
        
        self.h_scroll = tk.Scrollbar(self.gantt_frame, orient="horizontal")
        self.h_scroll.pack(side="bottom", fill="x")
        
        self.canvas = tk.Canvas(self.gantt_frame, height=150, bg="#ffffff", 
                                highlightthickness=1, xscrollcommand=self.h_scroll.set)
        self.canvas.pack(side="top", fill="x")
        self.h_scroll.config(command=self.canvas.xview)
        
        # 4. Result table
        tk.Label(self.root, text="Detailed Results", font=('Arial', 12, 'bold')).pack(pady=5)
        self.result_tree = ttk.Treeview(self.root, columns=("PID", "Finish", "Turnaround", "Waiting"), show='headings', height=8)
        for col in ("PID", "Finish", "Turnaround", "Waiting"):
            self.result_tree.heading(col, text=col)
            self.result_tree.column(col, width=100, anchor="center")
        self.result_tree.pack(pady=10, fill="x", padx=20)

    def setup_input_table(self):
        table_frame = tk.Frame(self.root)
        table_frame.pack(pady=10)
        self.tree = ttk.Treeview(table_frame, columns=("PID", "Arrival", "Burst", "Priority"), show='headings', height=6)
        for col in ("PID", "Arrival", "Burst", "Priority"):
            self.tree.heading(col, text=col)
            self.tree.column(col, width=100, anchor="center")
        self.tree.pack(side="left")

    def setup_controls(self):
        ctrl = tk.LabelFrame(self.root, text="Controls & Data Entry", padx=10, pady=10)
        ctrl.pack(pady=10, padx=20, fill="x")

        # Row 1: Data Entry
        r1 = tk.Frame(ctrl); r1.pack(fill="x", pady=2)
        tk.Label(r1, text="PID:").pack(side="left")
        self.entry_pid = tk.Entry(r1, width=5); self.entry_pid.pack(side="left", padx=5)
        tk.Label(r1, text="Arrival:").pack(side="left")
        self.entry_arrival = tk.Entry(r1, width=5); self.entry_arrival.pack(side="left", padx=5)
        tk.Label(r1, text="Burst:").pack(side="left")
        self.entry_burst = tk.Entry(r1, width=5); self.entry_burst.pack(side="left", padx=5)
        tk.Label(r1, text="Priority:").pack(side="left")
        self.entry_priority = tk.Entry(r1, width=5); self.entry_priority.pack(side="left", padx=5)
        tk.Button(r1, text="Add Process", command=self.add_process, bg="#e1f5fe").pack(side="left", padx=5)
        tk.Button(r1, text="Delete Selected", command=self.delete_process, bg="#ffebee").pack(side="left")

        # Row 2: Algorithm selection
        r2 = tk.Frame(ctrl); r2.pack(fill="x", pady=10)
        tk.Label(r2, text="Algorithm:").pack(side="left")
        self.algo_combo = ttk.Combobox(r2, values=[
            "FCFS", "SJF (Non-Pre)", "SJF (Preemptive/SRTF)", 
            "Priority (Non-Pre)", "Priority (Preemptive)", 
            "Round Robin", "MLFQ"
        ], state="readonly", width=25)
        self.algo_combo.pack(side="left", padx=5); self.algo_combo.current(0)
        
        tk.Label(r2, text="Time Quantum:").pack(side="left", padx=5)
        self.entry_quantum = tk.Entry(r2, width=5); self.entry_quantum.insert(0, "2")
        self.entry_quantum.pack(side="left")

        tk.Button(r2, text="RUN SIMULATION", command=self.run_simulation, bg="#4caf50", fg="white", font=('Arial', 10, 'bold')).pack(side="right", padx=10)

    # --- Scheduling Algorithms ---

    def algo_sjf_preemptive(self, processes):
        log, curr_time, completed = [], 0, 0
        n = len(processes)
        last_pid, start_seg = None, 0
        while completed < n:
            available = [p for p in processes if p.arrival <= curr_time and p.remaining_time > 0]
            if not available:
                if last_pid is not None:
                    log.append((last_pid, start_seg, curr_time))
                    last_pid = None
                curr_time += 1
                continue
            p = min(available, key=lambda x: x.remaining_time)
            if last_pid != p.pid:
                if last_pid is not None: log.append((last_pid, start_seg, curr_time))
                start_seg, last_pid = curr_time, p.pid
            p.remaining_time -= 1
            curr_time += 1
            if p.remaining_time == 0:
                p.completion_time = curr_time
                completed += 1
                log.append((p.pid, start_seg, curr_time))
                last_pid = None
        return log

    def algo_priority_preemptive(self, processes):
        log, curr_time, completed = [], 0, 0
        n = len(processes)
        last_pid, start_seg = None, 0
        while completed < n:
            available = [p for p in processes if p.arrival <= curr_time and p.remaining_time > 0]
            if not available:
                if last_pid is not None:
                    log.append((last_pid, start_seg, curr_time))
                    last_pid = None
                curr_time += 1
                continue
            p = min(available, key=lambda x: x.priority)
            if last_pid != p.pid:
                if last_pid is not None: log.append((last_pid, start_seg, curr_time))
                start_seg, last_pid = curr_time, p.pid
            p.remaining_time -= 1
            curr_time += 1
            if p.remaining_time == 0:
                p.completion_time = curr_time
                completed += 1
                log.append((p.pid, start_seg, curr_time))
                last_pid = None
        return log

    def algo_mlfq(self, processes, q1=4, q2=8):
        log, curr_time, completed = [], 0, 0
        n = len(processes)
        queues = [collections.deque(), collections.deque(), collections.deque()]
        procs = sorted(processes, key=lambda p: p.arrival)
        while completed < n:
            while procs and procs[0].arrival <= curr_time:
                queues[0].append(procs.pop(0))
            target_q = -1
            for i in range(3):
                if queues[i]:
                    target_q = i
                    break
            if target_q == -1:
                curr_time += 1
                continue
            p = queues[target_q].popleft()
            start = curr_time
            if target_q == 0:
                exec_time = min(p.remaining_time, q1)
                p.remaining_time -= exec_time
                curr_time += exec_time
                log.append((p.pid, start, curr_time))
                while procs and procs[0].arrival <= curr_time: queues[0].append(procs.pop(0))
                if p.remaining_time > 0: queues[1].append(p)
                else: p.completion_time = curr_time; completed += 1
            elif target_q == 1:
                exec_time = min(p.remaining_time, q2)
                p.remaining_time -= exec_time
                curr_time += exec_time
                log.append((p.pid, start, curr_time))
                while procs and procs[0].arrival <= curr_time: queues[0].append(procs.pop(0))
                if p.remaining_time > 0: queues[2].append(p)
                else: p.completion_time = curr_time; completed += 1
            else:
                curr_time += p.remaining_time
                p.remaining_time = 0
                log.append((p.pid, start, curr_time))
                p.completion_time = curr_time
                completed += 1
                while procs and procs[0].arrival <= curr_time: queues[0].append(procs.pop(0))
        return log

    # --- System Functions ---
    def add_process(self):
        try:
            p_id = self.entry_pid.get()
            if not p_id: raise ValueError
            self.tree.insert("", tk.END, values=(p_id, int(self.entry_arrival.get()), 
                                                int(self.entry_burst.get()), int(self.entry_priority.get() or 0)))
            # Clear entries after adding
            self.entry_pid.delete(0, tk.END)
            self.entry_arrival.delete(0, tk.END)
            self.entry_burst.delete(0, tk.END)
            self.entry_priority.delete(0, tk.END)
        except: 
            messagebox.showerror("Error", "Please enter a valid PID and numeric values for Arrival, Burst, and Priority!")

    def delete_process(self):
        selected_items = self.tree.selection()
        if not selected_items:
            messagebox.showwarning("Warning", "Please select a process to delete!")
            return
        for s in selected_items: self.tree.delete(s)

    def get_all_processes(self):
        return [Process(str(v[0]), int(v[1]), int(v[2]), int(v[3])) for v in [self.tree.item(i)['values'] for i in self.tree.get_children()]]

    def run_simulation(self):
        processes = self.get_all_processes()
        if not processes: 
            messagebox.showwarning("Notice", "Please add at least one process to the table!")
            return
        orig_bursts = {p.pid: p.burst for p in processes}
        algo = self.algo_combo.get()
        log = []
        
        if algo == "FCFS":
            processes.sort(key=lambda p: p.arrival)
            curr = 0
            for p in processes:
                if curr < p.arrival: curr = p.arrival
                log.append((p.pid, curr, curr + p.burst))
                curr += p.burst
                p.completion_time = curr
        elif algo == "SJF (Non-Pre)":
            curr, comp = 0, 0
            while comp < len(processes):
                avail = [p for p in processes if p.arrival <= curr and p.remaining_time > 0]
                if not avail: curr += 1; continue
                p = min(avail, key=lambda x: x.burst)
                log.append((p.pid, curr, curr + p.burst))
                curr += p.burst; p.completion_time = curr; p.remaining_time = 0; comp += 1
        elif "SRTF" in algo: log = self.algo_sjf_preemptive(processes)
        elif "Priority (Non-Pre)" in algo:
            curr, comp = 0, 0
            while comp < len(processes):
                avail = [p for p in processes if p.arrival <= curr and p.remaining_time > 0]
                if not avail: curr += 1; continue
                p = min(avail, key=lambda x: x.priority)
                log.append((p.pid, curr, curr + p.burst))
                curr += p.burst; p.completion_time = curr; p.remaining_time = 0; comp += 1
        elif "Priority (Preemptive)" in algo: log = self.algo_priority_preemptive(processes)
        elif "Round Robin" in algo: 
            try:
                q = int(self.entry_quantum.get())
                log = self.algo_mlfq(processes, q, 99999) # RR is treated as a 1-level MLFQ
            except:
                messagebox.showerror("Error", "Please enter a valid integer for Time Quantum!")
                return
        elif "MLFQ" in algo: log = self.algo_mlfq(processes)
        
        self.draw_gantt(log)
        self.update_results(processes, orig_bursts)

    def draw_gantt(self, log):
        self.canvas.delete("all")
        if not log: return
        scale = 25
        total_width = 60 + log[-1][2] * scale
        self.canvas.config(scrollregion=(0, 0, max(total_width, 1000), 150))
        
        for pid, s, e in log:
            x1, x2 = 30 + s*scale, 30 + e*scale
            # Assign colors based on PID hash
            color = ["#ffcdd2", "#c8e6c9", "#bbdefb", "#fff9c4", "#d1c4e9", "#e1f5fe"][hash(pid)%6]
            self.canvas.create_rectangle(x1, 30, x2, 80, fill=color, outline="#333", width=1)
            self.canvas.create_text((x1+x2)/2, 55, text=f"P{pid}", font=('Arial', 9, 'bold'))
            self.canvas.create_text(x1, 100, text=str(s), font=('Arial', 8))
        self.canvas.create_text(30 + log[-1][2]*scale, 100, text=str(log[-1][2]), font=('Arial', 8))

    def update_results(self, processes, original_bursts):
        for i in self.result_tree.get_children(): self.result_tree.delete(i)
        for p in processes:
            tr = p.completion_time - p.arrival
            wt = tr - original_bursts[p.pid]
            self.result_tree.insert("", tk.END, values=(p.pid, p.completion_time, tr, wt))

if __name__ == "__main__":
    root = tk.Tk(); app = SchedulerGUI(root); root.mainloop()