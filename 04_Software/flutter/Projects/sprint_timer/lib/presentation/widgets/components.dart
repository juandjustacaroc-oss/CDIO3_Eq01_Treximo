import 'package:flutter/material.dart';
import '../../core/theme/app_theme.dart';
import '../../domain/entities/entities.dart';
import 'package:intl/intl.dart';

// ─────────────────────────────────────────
// MetricCard
// ─────────────────────────────────────────
class MetricCard extends StatelessWidget {
  final String label;
  final String value;
  final String? subtitle;
  final Color accentColor;
  final String? trend; // e.g. "↑ +4.2%"
  final Color? trendColor;

  const MetricCard({
    super.key,
    required this.label,
    required this.value,
    this.subtitle,
    this.accentColor = AppColors.cyan,
    this.trend,
    this.trendColor,
  });

  @override
  Widget build(BuildContext context) {
    return Container(
      padding: const EdgeInsets.all(14),
      decoration: BoxDecoration(
        color: AppColors.surface,
        borderRadius: BorderRadius.circular(12),
        border: Border(left: BorderSide(color: accentColor, width: 3)),
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        mainAxisSize: MainAxisSize.min,
        children: [
          Text(label.toUpperCase(),
              style: const TextStyle(
                  fontSize: 10,
                  letterSpacing: 0.8,
                  fontWeight: FontWeight.w600,
                  color: AppColors.textTertiary)),
          const SizedBox(height: 6),
          Text(value,
              style: TextStyle(
                  fontFamily: 'JetBrainsMono',
                  fontSize: 22,
                  fontWeight: FontWeight.w700,
                  color: accentColor)),
          if (subtitle != null)
            Text(subtitle!,
                style: const TextStyle(fontSize: 11, color: AppColors.textTertiary)),
          if (trend != null) ...[
            const SizedBox(height: 2),
            Text(trend!,
                style: TextStyle(
                    fontSize: 11,
                    color: trendColor ?? AppColors.green,
                    fontWeight: FontWeight.w500)),
          ],
        ],
      ),
    );
  }
}

// ─────────────────────────────────────────
// StatusChip
// ─────────────────────────────────────────
class StatusChip extends StatelessWidget {
  final String label;
  final Color color;
  final bool dot;

  const StatusChip({
    super.key,
    required this.label,
    required this.color,
    this.dot = true,
  });

  @override
  Widget build(BuildContext context) {
    return Container(
      padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 5),
      decoration: BoxDecoration(
        color: color.withValues(alpha: 0.12),
        borderRadius: BorderRadius.circular(20),
        border: Border.all(color: color.withValues(alpha: 0.35), width: 0.8),
      ),
      child: Row(
        mainAxisSize: MainAxisSize.min,
        children: [
          if (dot) ...[
            Container(
              width: 6,
              height: 6,
              decoration: BoxDecoration(color: color, shape: BoxShape.circle),
            ),
            const SizedBox(width: 5),
          ],
          Text(label,
              style: TextStyle(fontSize: 12, fontWeight: FontWeight.w600, color: color)),
        ],
      ),
    );
  }
}

// ─────────────────────────────────────────
// SessionTile
// ─────────────────────────────────────────
class SessionTile extends StatelessWidget {
  final Session session;
  final String athleteName;
  final VoidCallback? onTap;

  const SessionTile({
    super.key,
    required this.session,
    required this.athleteName,
    this.onTap,
  });

  @override
  Widget build(BuildContext context) {
    final distColor = session.distanceMeters == 100 ? AppColors.cyan : AppColors.violet;
    final dateStr = DateFormat('dd/MM · HH:mm').format(session.datetimeStart);

    return GestureDetector(
      onTap: onTap,
      child: Container(
        margin: const EdgeInsets.only(bottom: 8),
        padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 14),
        decoration: BoxDecoration(
          color: AppColors.surface,
          borderRadius: BorderRadius.circular(12),
          border: Border.all(color: AppColors.border, width: 0.5),
        ),
        child: Row(
          children: [
            // Distance badge
            Container(
              width: 46,
              height: 46,
              decoration: BoxDecoration(
                color: distColor.withValues(alpha: 0.12),
                borderRadius: BorderRadius.circular(8),
              ),
              child: Center(
                child: Text('${session.distanceMeters}m',
                    style: TextStyle(
                        fontSize: 13,
                        fontWeight: FontWeight.w700,
                        color: distColor)),
              ),
            ),
            const SizedBox(width: 12),
            // Info
            Expanded(
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  Row(children: [
                    Text('$athleteName · #${session.athleteId}',
                        style: const TextStyle(
                            fontSize: 14,
                            fontWeight: FontWeight.w600,
                            color: AppColors.textPrimary)),
                    if (session.isPR) ...[
                      const SizedBox(width: 6),
                      StatusChip(label: '★ PR', color: AppColors.green, dot: false),
                    ],
                  ]),
                  const SizedBox(height: 3),
                  Text('$dateStr · ${session.bpmFinish} BPM · ECV ${session.ecv.toStringAsFixed(3)}',
                      style: const TextStyle(fontSize: 12, color: AppColors.textTertiary)),
                ],
              ),
            ),
            // Time
            Text(session.timeFormatted,
                style: TextStyle(
                    fontFamily: 'JetBrainsMono',
                    fontSize: 20,
                    fontWeight: FontWeight.w700,
                    color: session.isPR ? AppColors.green : AppColors.textPrimary)),
          ],
        ),
      ),
    );
  }
}

// ─────────────────────────────────────────
// FilterBar
// ─────────────────────────────────────────
class FilterBar extends StatelessWidget {
  final int? selectedAthleteId;
  final double? selectedDistance;
  final List<({int id, String name})> athletes;
  final ValueChanged<int?> onAthleteChanged;
  final ValueChanged<double?> onDistanceChanged;
  final VoidCallback? onClearFilters;

  const FilterBar({
    super.key,
    this.selectedAthleteId,
    this.selectedDistance,
    required this.athletes,
    required this.onAthleteChanged,
    required this.onDistanceChanged,
    this.onClearFilters,
  });

  @override
  Widget build(BuildContext context) {
    return SizedBox(
      height: 38,
      child: Row(
        children: [
          // Athlete filter
          Expanded(
            child: _FilterChip(
              label: selectedAthleteId == null
                  ? 'Todos los atletas'
                  : athletes.firstWhere((a) => a.id == selectedAthleteId, orElse: () => (id: 0, name: 'Atleta')).name,
              isActive: selectedAthleteId != null,
              onTap: () => _showAthleteSheet(context),
            ),
          ),
          const SizedBox(width: 8),
          // Distance filter
          _FilterChip(
            label: selectedDistance == null ? 'Distancia' : '${selectedDistance}m',
            isActive: selectedDistance != null,
            onTap: () => _showDistanceSheet(context),
          ),
          if (selectedAthleteId != null || selectedDistance != null) ...[
            const SizedBox(width: 8),
            GestureDetector(
              onTap: onClearFilters,
              child: Container(
                padding: const EdgeInsets.symmetric(horizontal: 10, vertical: 8),
                decoration: BoxDecoration(
                  color: AppColors.surfaceHigh,
                  borderRadius: BorderRadius.circular(8),
                  border: Border.all(color: AppColors.border, width: 0.5),
                ),
                child: const Text('✕',
                    style: TextStyle(fontSize: 12, color: AppColors.textSecondary)),
              ),
            ),
          ],
        ],
      ),
    );
  }

  void _showAthleteSheet(BuildContext context) {
    showModalBottomSheet(
      context: context,
      backgroundColor: AppColors.surface,
      shape: const RoundedRectangleBorder(
          borderRadius: BorderRadius.vertical(top: Radius.circular(16))),
      builder: (_) => Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          mainAxisSize: MainAxisSize.min,
          children: [
            const Text('Filtrar por atleta',
                style: TextStyle(fontWeight: FontWeight.w600, fontSize: 16, color: AppColors.textPrimary)),
            const SizedBox(height: 12),
            ListTile(
              title: const Text('Todos', style: TextStyle(color: AppColors.textPrimary)),
              onTap: () { Navigator.pop(context); onAthleteChanged(null); },
            ),
            for (final a in athletes)
              ListTile(
                title: Text(a.name, style: const TextStyle(color: AppColors.textPrimary)),
                subtitle: Text('#${a.id}', style: const TextStyle(color: AppColors.textTertiary)),
                trailing: selectedAthleteId == a.id
                    ? const Icon(Icons.check, color: AppColors.cyan)
                    : null,
                onTap: () { Navigator.pop(context); onAthleteChanged(a.id); },
              ),
          ],
        ),
      ),
    );
  }

  void _showDistanceSheet(BuildContext context) {
    showModalBottomSheet(
      context: context,
      backgroundColor: AppColors.surface,
      shape: const RoundedRectangleBorder(
          borderRadius: BorderRadius.vertical(top: Radius.circular(16))),
      builder: (_) => Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          mainAxisSize: MainAxisSize.min,
          children: [
            const Text('Filtrar por distancia',
                style: TextStyle(fontWeight: FontWeight.w600, fontSize: 16, color: AppColors.textPrimary)),
            const SizedBox(height: 12),
            ListTile(
              title: const Text('Todas', style: TextStyle(color: AppColors.textPrimary)),
              trailing: selectedDistance == null ? const Icon(Icons.check, color: AppColors.cyan) : null,
              onTap: () { Navigator.pop(context); onDistanceChanged(null); },
            ),
            ListTile(
              title: const Text('50.0m', style: TextStyle(color: AppColors.textPrimary)),
              trailing: selectedDistance == 50.0 ? const Icon(Icons.check, color: AppColors.cyan) : null,
              onTap: () { Navigator.pop(context); onDistanceChanged(50.0); },
            ),
            ListTile(
              title: const Text('100.0m', style: TextStyle(color: AppColors.textPrimary)),
              trailing: selectedDistance == 100.0 ? const Icon(Icons.check, color: AppColors.cyan) : null,
              onTap: () { Navigator.pop(context); onDistanceChanged(100.0); },
            ),
          ],
        ),
      ),
    );
  }
}

class _FilterChip extends StatelessWidget {
  final String label;
  final bool isActive;
  final VoidCallback onTap;

  const _FilterChip({required this.label, required this.isActive, required this.onTap});

  @override
  Widget build(BuildContext context) {
    return GestureDetector(
      onTap: onTap,
      child: Container(
        padding: const EdgeInsets.symmetric(horizontal: 12),
        decoration: BoxDecoration(
          color: isActive ? AppColors.cyan.withValues(alpha: 0.12) : AppColors.surface,
          borderRadius: BorderRadius.circular(8),
          border: Border.all(
            color: isActive ? AppColors.cyan.withValues(alpha: 0.4) : AppColors.border,
            width: 0.8,
          ),
        ),
        child: Align(
          alignment: Alignment.centerLeft,
          child: Text(label,
              overflow: TextOverflow.ellipsis,
              style: TextStyle(
                  fontSize: 13,
                  fontWeight: FontWeight.w500,
                  color: isActive ? AppColors.cyan : AppColors.textSecondary)),
        ),
      ),
    );
  }
}

// ─────────────────────────────────────────
// ConnectionPanel — muestra estado HW
// ─────────────────────────────────────────
class ConnectionPanel extends StatelessWidget {
  final List<({String name, bool ok})> devices;

  const ConnectionPanel({super.key, required this.devices});

  @override
  Widget build(BuildContext context) {
    return Container(
      padding: const EdgeInsets.all(14),
      decoration: BoxDecoration(
        color: AppColors.surface,
        borderRadius: BorderRadius.circular(12),
        border: Border.all(color: AppColors.border, width: 0.5),
      ),
      child: Column(
        children: devices.map((d) => Padding(
          padding: const EdgeInsets.symmetric(vertical: 5),
          child: Row(
            children: [
              Container(
                width: 8, height: 8,
                decoration: BoxDecoration(
                  color: d.ok ? AppColors.green : AppColors.red,
                  shape: BoxShape.circle,
                ),
              ),
              const SizedBox(width: 10),
              Text(d.name,
                  style: const TextStyle(fontSize: 14, color: AppColors.textPrimary)),
              const Spacer(),
              Text(d.ok ? 'Conectado' : 'Sin señal',
                  style: TextStyle(
                      fontSize: 12,
                      color: d.ok ? AppColors.green : AppColors.red)),
            ],
          ),
        )).toList(),
      ),
    );
  }
}
